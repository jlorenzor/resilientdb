#include "platform/consensus/ordering/hs2/hs2_timeout_certificate.h"

#include <algorithm>
#include <set>

namespace resdb {
namespace hs2 {

namespace {

int CalculateQuorumSize(int replica_count) {
  if (replica_count <= 0) {
    return 0;
  }
  const int f = (replica_count - 1) / 3;
  return (2 * f) + 1;
}

}  // namespace

Hs2TimeoutCollector::Hs2TimeoutCollector(int replica_count,
                                         int64_t target_view)
    : replica_count_(replica_count),
      quorum_size_(CalculateQuorumSize(replica_count)),
      target_view_(target_view) {}

int Hs2TimeoutCollector::ReplicaCount() const { return replica_count_; }

int Hs2TimeoutCollector::QuorumSize() const { return quorum_size_; }

int64_t Hs2TimeoutCollector::TargetView() const { return target_view_; }

bool Hs2TimeoutCollector::HasCertificate() const {
  return certificate_formed_;
}

const Hs2TimeoutCertificate& Hs2TimeoutCollector::Certificate() const {
  return certificate_;
}

std::vector<int> Hs2TimeoutCollector::Voters() const {
  std::vector<int> voters;
  voters.reserve(timeouts_by_replica_.size());
  for (const auto& entry : timeouts_by_replica_) {
    voters.push_back(entry.first);
  }
  return voters;
}

Hs2TimeoutRecordStatus Hs2TimeoutCollector::RecordTimeout(
    const Hs2TimeoutMessage& timeout) {
  const auto validation = ValidateTimeout(timeout);
  if (validation != Hs2TimeoutRecordStatus::kAccepted) {
    return validation;
  }
  if (timeouts_by_replica_.find(timeout.replica_id) !=
      timeouts_by_replica_.end()) {
    return Hs2TimeoutRecordStatus::kDuplicateReplica;
  }

  timeouts_by_replica_[timeout.replica_id] = timeout;
  if (!certificate_formed_ &&
      static_cast<int>(timeouts_by_replica_.size()) >= quorum_size_) {
    FormCertificate();
  }
  return Hs2TimeoutRecordStatus::kAccepted;
}

bool Hs2TimeoutCollector::IsValidCertificate(
    const Hs2TimeoutCertificate& certificate) const {
  if (certificate.view != target_view_ || target_view_ <= 0) {
    return false;
  }
  if (static_cast<int>(certificate.timeouts.size()) < quorum_size_) {
    return false;
  }

  std::set<int> unique_replicas;
  int64_t selected_high_qc_view = 0;
  std::string selected_high_qc_block_hash;
  for (const auto& timeout : certificate.timeouts) {
    if (ValidateTimeout(timeout) != Hs2TimeoutRecordStatus::kAccepted) {
      return false;
    }
    if (!unique_replicas.insert(timeout.replica_id).second) {
      return false;
    }
    if (timeout.high_qc_view > selected_high_qc_view) {
      selected_high_qc_view = timeout.high_qc_view;
      selected_high_qc_block_hash = timeout.high_qc_block_hash;
    }
  }
  if (static_cast<int>(unique_replicas.size()) < quorum_size_) {
    return false;
  }

  std::set<int> declared_voters(certificate.voters.begin(),
                                certificate.voters.end());
  if (declared_voters.size() != unique_replicas.size()) {
    return false;
  }
  if (declared_voters != unique_replicas) {
    return false;
  }
  if (certificate.high_qc_view != selected_high_qc_view) {
    return false;
  }
  return certificate.high_qc_block_hash == selected_high_qc_block_hash;
}

Hs2TimeoutRecordStatus Hs2TimeoutCollector::ValidateTimeout(
    const Hs2TimeoutMessage& timeout) const {
  if (timeout.replica_id <= 0 || timeout.replica_id > replica_count_) {
    return Hs2TimeoutRecordStatus::kInvalidReplica;
  }
  if (timeout.view <= 0 || target_view_ <= 0) {
    return Hs2TimeoutRecordStatus::kInvalidView;
  }
  if (timeout.view != target_view_) {
    return Hs2TimeoutRecordStatus::kWrongView;
  }
  if (timeout.high_qc_view < 0 || timeout.high_qc_view > timeout.view) {
    return Hs2TimeoutRecordStatus::kInvalidHighQc;
  }
  if (timeout.high_qc_view > 0 && timeout.high_qc_block_hash.empty()) {
    return Hs2TimeoutRecordStatus::kInvalidHighQc;
  }
  return Hs2TimeoutRecordStatus::kAccepted;
}

void Hs2TimeoutCollector::FormCertificate() {
  certificate_.view = target_view_;
  certificate_.timeouts.clear();
  certificate_.voters.clear();
  certificate_.high_qc_view = 0;
  certificate_.high_qc_block_hash.clear();

  for (const auto& entry : timeouts_by_replica_) {
    certificate_.timeouts.push_back(entry.second);
    certificate_.voters.push_back(entry.first);
    if (entry.second.high_qc_view > certificate_.high_qc_view) {
      certificate_.high_qc_view = entry.second.high_qc_view;
      certificate_.high_qc_block_hash = entry.second.high_qc_block_hash;
    }
  }
  std::sort(certificate_.voters.begin(), certificate_.voters.end());
  certificate_formed_ = IsValidCertificate(certificate_);
}

const char* TimeoutRecordStatusName(Hs2TimeoutRecordStatus status) {
  switch (status) {
    case Hs2TimeoutRecordStatus::kAccepted:
      return "ACCEPTED";
    case Hs2TimeoutRecordStatus::kDuplicateReplica:
      return "DUPLICATE_REPLICA";
    case Hs2TimeoutRecordStatus::kInvalidReplica:
      return "INVALID_REPLICA";
    case Hs2TimeoutRecordStatus::kInvalidView:
      return "INVALID_VIEW";
    case Hs2TimeoutRecordStatus::kWrongView:
      return "WRONG_VIEW";
    case Hs2TimeoutRecordStatus::kInvalidHighQc:
      return "INVALID_HIGH_QC";
  }
  return "UNKNOWN";
}

}  // namespace hs2
}  // namespace resdb
