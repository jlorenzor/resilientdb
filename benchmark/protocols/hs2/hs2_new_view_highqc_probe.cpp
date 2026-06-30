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
  info.set_port(21000 + id);
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
  timeout.signature = "local-timeout-signature-" + std::to_string(replica_id);
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
  message.signature = "local-new-view-signature-" + std::to_string(replica_id);
  return message;
}

}  // namespace

int main() {
  constexpr int kReplicaCount = 4;
  constexpr int64_t kNewView = 2;
  std::vector<resdb::ReplicaInfo> replicas;
  for (int id = 1; id <= kReplicaCount; ++id) {
    replicas.push_back(MakeReplica(id));
  }

  resdb::ResDBConfig config(replicas, replicas.front());
  resdb::SystemInfo system_info(config);
  resdb::hs2::Hs2Pacemaker pacemaker(config, &system_info);
  resdb::hs2::Hs2Consensus consensus(kReplicaCount);

  const auto timeout_certificate = MakeTimeoutCertificate();
  resdb::hs2::Hs2TimeoutCollector timeout_validator(kReplicaCount, 1);
  if (!timeout_validator.IsValidCertificate(timeout_certificate)) {
    return Fail("timeout certificate fixture is invalid");
  }

  const auto high_qc = MakeHighQc(1, "block-1");
  if (!consensus.IsValidQuorumCertificate(high_qc)) {
    return Fail("highQC fixture is invalid");
  }

  resdb::hs2::Hs2NewViewCollector collector(kReplicaCount, kNewView);
  if (collector.QuorumSize() != 3) {
    return Fail("unexpected new-view quorum size");
  }

  auto wrong_view = NewViewFrom(2, 3, timeout_certificate, high_qc);
  if (collector.RecordNewView(wrong_view) !=
      resdb::hs2::Hs2NewViewRecordStatus::kWrongView) {
    return Fail("wrong-view new-view accepted");
  }

  auto invalid_tc = timeout_certificate;
  invalid_tc.view = 7;
  auto bad_tc_message = NewViewFrom(2, kNewView, invalid_tc, high_qc);
  if (collector.RecordNewView(bad_tc_message) !=
      resdb::hs2::Hs2NewViewRecordStatus::kInvalidTimeoutCertificate) {
    return Fail("new-view with invalid TC accepted");
  }

  auto old_high_qc = MakeHighQc(1, "block-1");
  auto first = NewViewFrom(2, kNewView, timeout_certificate, old_high_qc);
  if (collector.RecordNewView(first) !=
      resdb::hs2::Hs2NewViewRecordStatus::kAccepted) {
    return Fail("first new-view rejected");
  }
  if (collector.HasQuorum()) {
    return Fail("new-view quorum formed too early");
  }

  if (collector.RecordNewView(first) !=
      resdb::hs2::Hs2NewViewRecordStatus::kDuplicateReplica) {
    return Fail("duplicate new-view accepted");
  }

  resdb::hs2::Hs2QuorumCertificate empty_qc;
  auto second = NewViewFrom(3, kNewView, timeout_certificate, empty_qc);
  if (collector.RecordNewView(second) !=
      resdb::hs2::Hs2NewViewRecordStatus::kAccepted) {
    return Fail("second new-view rejected");
  }
  if (collector.HasQuorum()) {
    return Fail("new-view quorum formed with only two messages");
  }

  auto third = NewViewFrom(4, kNewView, timeout_certificate, high_qc);
  if (collector.RecordNewView(third) !=
      resdb::hs2::Hs2NewViewRecordStatus::kAccepted) {
    return Fail("third new-view rejected");
  }
  if (!collector.HasQuorum()) {
    return Fail("new-view quorum not formed");
  }
  if (!collector.HasSelectedHighQc()) {
    return Fail("new-view did not select highQC");
  }

  const auto& selected_high_qc = collector.SelectedHighQc();
  if (selected_high_qc.view != 1 || selected_high_qc.block_hash != "block-1") {
    return Fail("unexpected selected highQC");
  }

  if (!pacemaker.AdvanceView(kNewView)) {
    return Fail("pacemaker did not advance to new view");
  }
  if (pacemaker.CurrentLeader() != 2) {
    return Fail("unexpected new-view leader");
  }

  consensus.SetLockedQc(selected_high_qc);
  resdb::hs2::Hs2Block safe_block;
  safe_block.height = 2;
  safe_block.view = kNewView;
  safe_block.block_hash = "block-2";
  safe_block.parent_hash = "block-1";
  safe_block.payload_digest = "sha256:new-view";
  safe_block.proposer_id = pacemaker.CurrentLeader();
  if (!collector.IsSafeProposal(safe_block)) {
    return Fail("safe proposal rejected");
  }
  if (!consensus.IsSafeProposal(safe_block, selected_high_qc)) {
    return Fail("consensus rejected safe proposal");
  }

  resdb::hs2::Hs2Block unsafe_block = safe_block;
  unsafe_block.parent_hash = "conflicting-parent";
  if (collector.IsSafeProposal(unsafe_block)) {
    return Fail("collector accepted unsafe proposal");
  }
  if (consensus.IsSafeProposal(unsafe_block, selected_high_qc)) {
    return Fail("consensus accepted unsafe proposal");
  }

  std::cout << "HS2_NEW_VIEW_FORMED new_view=" << collector.NewView()
            << " quorum=" << collector.QuorumSize()
            << " voters=2,3,4"
            << " selected_high_qc_view=" << selected_high_qc.view
            << " selected_high_qc_block=" << selected_high_qc.block_hash
            << " leader=" << pacemaker.CurrentLeader() << std::endl;
  std::cout << "HS2_HIGH_QC_SELECTED view=" << selected_high_qc.view
            << " block=" << selected_high_qc.block_hash << std::endl;
  std::cout << "hs2 new-view highqc ok" << std::endl;
  return 0;
}
