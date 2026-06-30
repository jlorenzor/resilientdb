#include <iostream>
#include <string>

#include "common/crypto/signature_verifier.h"
#include "platform/consensus/ordering/hs2/hs2_payload_binding.h"
#include "platform/proto/resdb.pb.h"

namespace {

int Fail(const char* message) {
  std::cerr << message << std::endl;
  return 1;
}

resdb::Request MakeRequest() {
  resdb::Request request;
  request.set_type(resdb::Request::TYPE_NEW_TXNS);
  request.set_data("chatay-hs2-binding-payload");
  request.set_hash(resdb::SignatureVerifier::CalculateHash(request.data()));
  request.set_seq(12);
  request.set_current_view(3);
  request.set_sender_id(2);
  request.set_proxy_id(1);
  return request;
}

resdb::hs2::Hs2Block MakeBlock() {
  resdb::hs2::Hs2Block block;
  block.height = 12;
  block.view = 3;
  block.parent_hash = "parent";
  block.payload_digest = "payload";
  block.proposer_id = 2;
  block.block_hash = "block";
  return block;
}

resdb::hs2::Hs2QuorumCertificate MakeQc(resdb::hs2::Hs2Phase phase) {
  resdb::hs2::Hs2QuorumCertificate qc;
  qc.height = 12;
  qc.view = 3;
  qc.block_hash = "block";
  qc.phase = phase;
  qc.voters = {1, 2, 3};
  qc.vote_signatures = {"sig1", "sig2", "sig3"};
  qc.proof_digest =
      phase == resdb::hs2::Hs2Phase::kPhase1 ? "phase1-proof"
                                             : "phase2-proof";
  return qc;
}

}  // namespace

int main() {
  resdb::hs2::Hs2CommitProofBuilder builder;
  auto request = MakeRequest();
  const auto proof = builder.Serialize(request, MakeBlock(),
                                       MakeQc(resdb::hs2::Hs2Phase::kPhase1),
                                       MakeQc(resdb::hs2::Hs2Phase::kPhase2));

  std::string reason;
  if (!builder.VerifyForRequest(request, proof, &reason)) {
    return Fail("valid proof did not verify");
  }

  auto tampered = request;
  tampered.set_data("tampered-payload");
  if (builder.VerifyForRequest(tampered, proof, &reason)) {
    return Fail("tampered request verified against original proof");
  }

  if (proof.find("payload_digest=") == std::string::npos ||
      proof.find("request_hash=") == std::string::npos ||
      proof.find("execution_digest=") == std::string::npos ||
      proof.find("binding_digest=") == std::string::npos) {
    return Fail("proof is missing binding fields");
  }

  std::cout << "hs2 payload binding ok" << std::endl;
  return 0;
}
