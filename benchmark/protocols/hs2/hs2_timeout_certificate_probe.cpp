#include <iostream>
#include <string>
#include <vector>

#include "platform/config/resdb_config.h"
#include "platform/consensus/execution/system_info.h"
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
  info.set_port(20000 + id);
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
  timeout.signature = "local-probe-signature-" + std::to_string(replica_id);
  return timeout;
}

}  // namespace

int main() {
  constexpr int kReplicaCount = 4;
  constexpr int64_t kTimedOutView = 1;
  std::vector<resdb::ReplicaInfo> replicas;
  for (int id = 1; id <= kReplicaCount; ++id) {
    replicas.push_back(MakeReplica(id));
  }

  resdb::ResDBConfig config(replicas, replicas.front());
  resdb::SystemInfo system_info(config);
  resdb::hs2::Hs2Pacemaker pacemaker(config, &system_info);
  resdb::hs2::Hs2TimeoutCollector collector(kReplicaCount, kTimedOutView);

  if (collector.QuorumSize() != 3) {
    return Fail("unexpected timeout quorum size");
  }
  if (pacemaker.CurrentView() != kTimedOutView ||
      pacemaker.CurrentLeader() != 1) {
    return Fail("unexpected initial pacemaker state");
  }

  const auto invalid_replica =
      collector.RecordTimeout(TimeoutFrom(0, kTimedOutView, 0, "genesis"));
  if (invalid_replica !=
      resdb::hs2::Hs2TimeoutRecordStatus::kInvalidReplica) {
    return Fail("invalid replica accepted");
  }

  const auto wrong_view =
      collector.RecordTimeout(TimeoutFrom(2, kTimedOutView + 1, 0, "genesis"));
  if (wrong_view != resdb::hs2::Hs2TimeoutRecordStatus::kWrongView) {
    return Fail("wrong-view timeout accepted");
  }

  const auto first =
      collector.RecordTimeout(TimeoutFrom(2, kTimedOutView, 0, "genesis"));
  if (first != resdb::hs2::Hs2TimeoutRecordStatus::kAccepted) {
    return Fail("first timeout rejected");
  }
  if (collector.HasCertificate()) {
    return Fail("certificate formed before quorum");
  }

  const auto duplicate =
      collector.RecordTimeout(TimeoutFrom(2, kTimedOutView, 0, "genesis"));
  if (duplicate != resdb::hs2::Hs2TimeoutRecordStatus::kDuplicateReplica) {
    return Fail("duplicate timeout accepted");
  }

  const auto second =
      collector.RecordTimeout(TimeoutFrom(3, kTimedOutView, 1, "block-1"));
  if (second != resdb::hs2::Hs2TimeoutRecordStatus::kAccepted) {
    return Fail("second timeout rejected");
  }
  if (collector.HasCertificate()) {
    return Fail("certificate formed with only two timeouts");
  }

  const auto third =
      collector.RecordTimeout(TimeoutFrom(4, kTimedOutView, 1, "block-1"));
  if (third != resdb::hs2::Hs2TimeoutRecordStatus::kAccepted) {
    return Fail("third timeout rejected");
  }
  if (!collector.HasCertificate()) {
    return Fail("certificate not formed at quorum");
  }

  const auto& certificate = collector.Certificate();
  if (!collector.IsValidCertificate(certificate)) {
    return Fail("formed certificate is invalid");
  }
  if (certificate.view != kTimedOutView) {
    return Fail("certificate view mismatch");
  }
  if (certificate.timeouts.size() != 3 || certificate.voters.size() != 3) {
    return Fail("certificate quorum content mismatch");
  }
  if (certificate.high_qc_view != 1 ||
      certificate.high_qc_block_hash != "block-1") {
    return Fail("certificate highQC selection mismatch");
  }

  resdb::hs2::Hs2TimeoutCertificate insufficient = certificate;
  insufficient.timeouts.pop_back();
  insufficient.voters.pop_back();
  if (collector.IsValidCertificate(insufficient)) {
    return Fail("insufficient certificate accepted");
  }

  resdb::hs2::Hs2TimeoutCertificate duplicate_cert = certificate;
  duplicate_cert.timeouts[2].replica_id = duplicate_cert.timeouts[0].replica_id;
  if (collector.IsValidCertificate(duplicate_cert)) {
    return Fail("duplicate-voter certificate accepted");
  }

  resdb::hs2::Hs2TimeoutCertificate wrong_view_cert = certificate;
  wrong_view_cert.view = kTimedOutView + 1;
  if (collector.IsValidCertificate(wrong_view_cert)) {
    return Fail("wrong-view certificate accepted");
  }

  if (!pacemaker.AdvanceView(certificate.view + 1)) {
    return Fail("pacemaker did not advance after timeout certificate");
  }
  if (pacemaker.CurrentView() != 2 || pacemaker.CurrentLeader() != 2) {
    return Fail("timeout certificate did not select expected next leader");
  }

  std::cout << "HS2_TIMEOUT_CERTIFICATE_FORMED view=" << certificate.view
            << " quorum=" << collector.QuorumSize()
            << " voters=2,3,4"
            << " high_qc_view=" << certificate.high_qc_view
            << " high_qc_block=" << certificate.high_qc_block_hash
            << " next_view=" << pacemaker.CurrentView()
            << " next_leader=" << pacemaker.CurrentLeader() << std::endl;
  std::cout << "hs2 timeout certificate ok" << std::endl;
  return 0;
}
