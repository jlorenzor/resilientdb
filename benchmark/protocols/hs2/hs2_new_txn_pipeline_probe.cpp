#include <iostream>
#include <string>
#include <vector>

#include "common/crypto/signature_verifier.h"
#include "platform/config/resdb_config.h"
#include "platform/consensus/ordering/hs2/hs2_new_txn_pipeline.h"
#include "platform/proto/resdb.pb.h"

namespace {

int Fail(const char* message) {
  std::cerr << message << std::endl;
  return 1;
}

resdb::ReplicaInfo MakeReplica(int id) {
  resdb::ReplicaInfo info;
  info.set_id(id);
  info.set_ip("127.0.0.1");
  info.set_port(26000 + id);
  return info;
}

resdb::Request MakeNewTxnRequest(uint64_t seq) {
  resdb::Request request;
  request.set_type(resdb::Request::TYPE_NEW_TXNS);
  request.set_data("chatay-hs2-new-txn-pipeline-payload");
  request.set_hash(resdb::SignatureVerifier::CalculateHash(request.data()));
  request.set_seq(seq);
  request.set_current_view(2);
  request.set_sender_id(2);
  request.set_proxy_id(1);
  return request;
}

}  // namespace

int main() {
  std::vector<resdb::ReplicaInfo> replicas;
  for (int id = 1; id <= 4; ++id) {
    replicas.push_back(MakeReplica(id));
  }

  resdb::ResDBConfig config(replicas, replicas.front());
  config.SetSignatureVerifierEnabled(false);
  config.SetHeartBeatEnabled(false);
  config.SetTestMode(true);

  resdb::hs2::Hs2NewTxnPipeline pipeline(config);
  auto request = MakeNewTxnRequest(/*seq=*/7);
  const auto certification = pipeline.Certify(&request, /*leader_id=*/2,
                                              /*view=*/2);

  if (!certification.committed) {
    return Fail("new txn request was not certified");
  }
  if (certification.block.height != 7 || certification.block.view != 2) {
    return Fail("unexpected block height/view");
  }
  if (certification.phase1_qc.voters.size() != 3 ||
      certification.phase2_qc.voters.size() != 3) {
    return Fail("unexpected quorum size");
  }
  if (!request.has_committed_certs() ||
      request.committed_certs().committed_certs_size() != 1) {
    return Fail("commit proof was not attached");
  }
  const auto& proof = request.committed_certs().committed_certs(0);
  if (proof.signature().find("hs2-alpha-commit") == std::string::npos) {
    return Fail("unexpected commit proof marker");
  }

  auto duplicate_request = MakeNewTxnRequest(/*seq=*/7);
  const auto duplicate = pipeline.Certify(&duplicate_request, /*leader_id=*/2,
                                          /*view=*/2);
  if (!duplicate.committed ||
      duplicate.reason.find("already committed") == std::string::npos) {
    return Fail("duplicate request was not handled idempotently");
  }

  auto next_request = MakeNewTxnRequest(/*seq=*/8);
  const auto next = pipeline.Certify(&next_request, /*leader_id=*/2,
                                     /*view=*/2);
  if (!next.committed || next.block.parent_hash != certification.block.block_hash) {
    return Fail("next request did not extend the committed block");
  }

  auto rejected_request = MakeNewTxnRequest(/*seq=*/9);
  const auto rejected = pipeline.Certify(&rejected_request, /*leader_id=*/9,
                                         /*view=*/2);
  if (rejected.committed) {
    return Fail("invalid leader was certified");
  }

  std::cout << "hs2 new txn pipeline ok" << std::endl;
  return 0;
}
