#include "platform/consensus/ordering/hs2/hs2_new_view.h"

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

bool IsEmptyHighQc(const Hs2QuorumCertificate& qc) {
  return qc.height == 0 && qc.view == 0 && qc.block_hash.empty() &&
         qc.voters.empty();
}

}  // namespace

Hs2NewViewCollector::Hs2NewViewCollector(int replica_count, int64_t new_view)
    : replica_count_(replica_count),
      quorum_size_(CalculateQuorumSize(replica_count)),
      new_view_(new_view),
      consensus_(replica_count) {}

int Hs2NewViewCollector::ReplicaCount() const { return replica_count_; }

int Hs2NewViewCollector::QuorumSize() const { return quorum_size_; }

int64_t Hs2NewViewCollector::NewView() const { return new_view_; }

bool Hs2NewViewCollector::HasQuorum() const { return quorum_formed_; }

bool Hs2NewViewCollector::HasSelectedHighQc() const {
  return selected_high_qc_.view > 0 && !selected_high_qc_.block_hash.empty();
}

const Hs2QuorumCertificate& Hs2NewViewCollector::SelectedHighQc() const {
  return selected_high_qc_;
}

std::vector<int> Hs2NewViewCollector::Voters() const {
  std::vector<int> voters;
  voters.reserve(messages_by_replica_.size());
  for (const auto& entry : messages_by_replica_) {
    voters.push_back(entry.first);
  }
  return voters;
}

Hs2NewViewRecordStatus Hs2NewViewCollector::RecordNewView(
    const Hs2NewViewMessage& message) {
  const auto validation = ValidateMessage(message);
  if (validation != Hs2NewViewRecordStatus::kAccepted) {
    return validation;
  }
  if (messages_by_replica_.find(message.replica_id) !=
      messages_by_replica_.end()) {
    return Hs2NewViewRecordStatus::kDuplicateReplica;
  }

  messages_by_replica_[message.replica_id] = message;
  if (!IsEmptyHighQc(message.high_qc) &&
      (selected_high_qc_.view == 0 ||
       message.high_qc.view > selected_high_qc_.view)) {
    selected_high_qc_ = message.high_qc;
  }
  if (!quorum_formed_ &&
      static_cast<int>(messages_by_replica_.size()) >= quorum_size_) {
    quorum_formed_ = true;
  }
  return Hs2NewViewRecordStatus::kAccepted;
}

bool Hs2NewViewCollector::IsSafeProposal(const Hs2Block& block) const {
  if (!quorum_formed_ || !HasSelectedHighQc()) {
    return false;
  }
  if (block.view != new_view_) {
    return false;
  }
  if (block.parent_hash != selected_high_qc_.block_hash) {
    return false;
  }
  return consensus_.IsSafeProposal(block, selected_high_qc_);
}

Hs2NewViewRecordStatus Hs2NewViewCollector::ValidateMessage(
    const Hs2NewViewMessage& message) const {
  if (message.replica_id <= 0 || message.replica_id > replica_count_) {
    return Hs2NewViewRecordStatus::kInvalidReplica;
  }
  if (new_view_ <= 1 || message.new_view <= 1) {
    return Hs2NewViewRecordStatus::kInvalidView;
  }
  if (message.new_view != new_view_) {
    return Hs2NewViewRecordStatus::kWrongView;
  }
  const int64_t timed_out_view = new_view_ - 1;
  Hs2TimeoutCollector timeout_validator(replica_count_, timed_out_view);
  if (!timeout_validator.IsValidCertificate(message.timeout_certificate)) {
    return Hs2NewViewRecordStatus::kInvalidTimeoutCertificate;
  }
  if (!IsEmptyHighQc(message.high_qc) &&
      !consensus_.IsValidQuorumCertificate(message.high_qc)) {
    return Hs2NewViewRecordStatus::kInvalidHighQc;
  }
  if (!IsEmptyHighQc(message.high_qc) &&
      message.high_qc.view > timed_out_view) {
    return Hs2NewViewRecordStatus::kInvalidHighQc;
  }
  return Hs2NewViewRecordStatus::kAccepted;
}

const char* NewViewRecordStatusName(Hs2NewViewRecordStatus status) {
  switch (status) {
    case Hs2NewViewRecordStatus::kAccepted:
      return "ACCEPTED";
    case Hs2NewViewRecordStatus::kDuplicateReplica:
      return "DUPLICATE_REPLICA";
    case Hs2NewViewRecordStatus::kInvalidReplica:
      return "INVALID_REPLICA";
    case Hs2NewViewRecordStatus::kInvalidView:
      return "INVALID_VIEW";
    case Hs2NewViewRecordStatus::kWrongView:
      return "WRONG_VIEW";
    case Hs2NewViewRecordStatus::kInvalidTimeoutCertificate:
      return "INVALID_TIMEOUT_CERTIFICATE";
    case Hs2NewViewRecordStatus::kInvalidHighQc:
      return "INVALID_HIGH_QC";
  }
  return "UNKNOWN";
}

}  // namespace hs2
}  // namespace resdb
