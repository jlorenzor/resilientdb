#include <iostream>
#include <string>
#include <vector>

#include "platform/config/resdb_config.h"
#include "platform/consensus/execution/system_info.h"
#include "platform/consensus/ordering/hs2/hs2_consensus.h"
#include "platform/consensus/ordering/hs2/hs2_new_view.h"
#include "platform/consensus/ordering/hs2/hs2_pacemaker.h"
#include "platform/consensus/ordering/hs2/hs2_timeout_certificate.h"
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
  info.set_port(22000 + id);
  return info;
}

resdb::hs2::Hs2TimeoutMessage TimeoutFrom(int replica_id, int64_t view,
                                          int64_t high_qc_view,
                                          const std::string& high_qc_hash) {
  resdb::hs2::Hs2TimeoutMessage timeout;
  timeout.replica_id = replica_id;
  timeout.view = view;
  timeout.high_qc_view = high_qc_view;
  timeout.high_qc_block_hash = high_qc_hash;
  timeout.signature = "negative-timeout-signature-" + std::to_string(replica_id);
  return timeout;
}

resdb::hs2::Hs2TimeoutCertificate MakeTimeoutCertificate() {
  resdb::hs2::Hs2TimeoutCollector collector(/*replica_count=*/4,
                                            /*target_view=*/1);
  collector.RecordTimeout(TimeoutFrom(2, 1, 1, "block-1"));
  collector.RecordTimeout(TimeoutFrom(3, 1, 0, "genesis"));
  collector.RecordTimeout(TimeoutFrom(4, 1, 1, "block-1"));
  return collector.Certificate();
}

resdb::hs2::Hs2QuorumCertificate MakeHighQc(int64_t view,
                                            const std::string& block_hash) {
  resdb::hs2::Hs2QuorumCertificate qc;
  qc.height = view;
  qc.view = view;
  qc.block_hash = block_hash;
  qc.phase = resdb::hs2::Hs2Phase::kPhase2;
  qc.voters = {1, 2, 3};
  return qc;
}

resdb::hs2::Hs2NewViewMessage NewViewFrom(
    int replica_id, int64_t new_view,
    const resdb::hs2::Hs2TimeoutCertificate& timeout_certificate,
    const resdb::hs2::Hs2QuorumCertificate& high_qc) {
  resdb::hs2::Hs2NewViewMessage message;
  message.replica_id = replica_id;
  message.new_view = new_view;
  message.timeout_certificate = timeout_certificate;
  message.high_qc = high_qc;
  message.signature = "negative-new-view-signature-" + std::to_string(replica_id);
  return message;
}

}  // namespace

int main() {
  constexpr int kReplicaCount = 4;
  int passed_cases = 0;

  resdb::hs2::Hs2Consensus consensus(kReplicaCount);

  resdb::hs2::Hs2QuorumCertificate valid_qc = MakeHighQc(1, "block-1");
  if (!consensus.IsValidQuorumCertificate(valid_qc)) {
    return Fail("fixture valid QC rejected");
  }

  resdb::hs2::Hs2QuorumCertificate duplicate_qc = valid_qc;
  duplicate_qc.voters = {1, 1, 2};
  if (consensus.IsValidQuorumCertificate(duplicate_qc)) {
    return Fail("duplicate voter QC accepted");
  }
  ++passed_cases;

  resdb::hs2::Hs2QuorumCertificate insufficient_qc = valid_qc;
  insufficient_qc.voters = {1, 2};
  if (consensus.IsValidQuorumCertificate(insufficient_qc)) {
    return Fail("insufficient quorum QC accepted");
  }
  ++passed_cases;

  resdb::hs2::Hs2Vote vote;
  vote.replica_id = 1;
  vote.height = 2;
  vote.view = 2;
  vote.block_hash = "block-2a";
  vote.phase = resdb::hs2::Hs2Phase::kPhase1;
  if (!consensus.RecordVote(vote)) {
    return Fail("fixture vote rejected");
  }
  resdb::hs2::Hs2Vote conflicting_vote = vote;
  conflicting_vote.block_hash = "block-2b";
  if (consensus.RecordVote(conflicting_vote)) {
    return Fail("conflicting duplicate vote accepted");
  }
  ++passed_cases;

  resdb::hs2::Hs2TimeoutCollector timeout_validator(kReplicaCount, 1);
  const auto valid_tc = MakeTimeoutCertificate();
  if (!timeout_validator.IsValidCertificate(valid_tc)) {
    return Fail("fixture valid TC rejected");
  }

  auto insufficient_tc = valid_tc;
  insufficient_tc.timeouts.pop_back();
  insufficient_tc.voters.pop_back();
  if (timeout_validator.IsValidCertificate(insufficient_tc)) {
    return Fail("insufficient quorum TC accepted");
  }
  ++passed_cases;

  auto duplicate_tc = valid_tc;
  duplicate_tc.timeouts[2].replica_id = duplicate_tc.timeouts[0].replica_id;
  if (timeout_validator.IsValidCertificate(duplicate_tc)) {
    return Fail("duplicate timeout TC accepted");
  }
  ++passed_cases;

  resdb::hs2::Hs2NewViewCollector new_view_collector(kReplicaCount, 2);
  auto invalid_tc_message = NewViewFrom(2, 2, insufficient_tc, valid_qc);
  if (new_view_collector.RecordNewView(invalid_tc_message) !=
      resdb::hs2::Hs2NewViewRecordStatus::kInvalidTimeoutCertificate) {
    return Fail("new-view with insufficient TC accepted");
  }
  ++passed_cases;

  resdb::hs2::Hs2QuorumCertificate future_high_qc = MakeHighQc(2, "block-2");
  auto future_high_qc_message = NewViewFrom(2, 2, valid_tc, future_high_qc);
  if (new_view_collector.RecordNewView(future_high_qc_message) !=
      resdb::hs2::Hs2NewViewRecordStatus::kInvalidHighQc) {
    return Fail("new-view with future highQC accepted");
  }
  ++passed_cases;

  auto wrong_view_message = NewViewFrom(2, 3, valid_tc, valid_qc);
  if (new_view_collector.RecordNewView(wrong_view_message) !=
      resdb::hs2::Hs2NewViewRecordStatus::kWrongView) {
    return Fail("new-view wrong view accepted");
  }
  ++passed_cases;

  if (new_view_collector.RecordNewView(NewViewFrom(2, 2, valid_tc, valid_qc)) !=
      resdb::hs2::Hs2NewViewRecordStatus::kAccepted) {
    return Fail("valid new-view from replica 2 rejected");
  }
  if (new_view_collector.RecordNewView(NewViewFrom(2, 2, valid_tc, valid_qc)) !=
      resdb::hs2::Hs2NewViewRecordStatus::kDuplicateReplica) {
    return Fail("duplicate new-view accepted");
  }
  ++passed_cases;

  if (new_view_collector.RecordNewView(NewViewFrom(3, 2, valid_tc, valid_qc)) !=
      resdb::hs2::Hs2NewViewRecordStatus::kAccepted) {
    return Fail("valid new-view from replica 3 rejected");
  }
  if (new_view_collector.RecordNewView(NewViewFrom(4, 2, valid_tc, valid_qc)) !=
      resdb::hs2::Hs2NewViewRecordStatus::kAccepted) {
    return Fail("valid new-view from replica 4 rejected");
  }
  if (!new_view_collector.HasQuorum()) {
    return Fail("new-view quorum did not form without leader");
  }
  ++passed_cases;

  resdb::hs2::Hs2Block stale_view_block;
  stale_view_block.height = 2;
  stale_view_block.view = 1;
  stale_view_block.block_hash = "stale";
  stale_view_block.parent_hash = "block-1";
  stale_view_block.payload_digest = "sha256:stale";
  stale_view_block.proposer_id = 2;
  if (new_view_collector.IsSafeProposal(stale_view_block)) {
    return Fail("stale-view proposal accepted");
  }
  ++passed_cases;

  resdb::hs2::Hs2Block conflicting_block;
  conflicting_block.height = 2;
  conflicting_block.view = 2;
  conflicting_block.block_hash = "conflict";
  conflicting_block.parent_hash = "other-parent";
  conflicting_block.payload_digest = "sha256:conflict";
  conflicting_block.proposer_id = 2;
  if (new_view_collector.IsSafeProposal(conflicting_block)) {
    return Fail("conflicting proposal accepted");
  }
  ++passed_cases;

  std::vector<resdb::ReplicaInfo> replicas;
  for (int id = 1; id <= kReplicaCount; ++id) {
    replicas.push_back(MakeReplica(id));
  }
  resdb::ResDBConfig config(replicas, replicas.front());
  resdb::SystemInfo system_info(config);
  resdb::hs2::Hs2Pacemaker pacemaker(config, &system_info);
  if (!pacemaker.AdvanceView(2) || pacemaker.CurrentLeader() != 2) {
    return Fail("pacemaker failed stopped-leader modeled advance");
  }
  ++passed_cases;

  std::cout << "HS2_NEGATIVE_SAFETY_PASSED cases=" << passed_cases
            << " duplicate_qc=reject"
            << " insufficient_qc=reject"
            << " conflicting_vote=reject"
            << " insufficient_tc=reject"
            << " duplicate_tc=reject"
            << " invalid_new_view=reject"
            << " unsafe_proposal=reject"
            << " leader_advance=ok" << std::endl;
  std::cout << "hs2 negative safety ok" << std::endl;
  return 0;
}
