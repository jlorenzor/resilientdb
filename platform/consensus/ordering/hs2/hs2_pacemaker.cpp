#include "platform/consensus/ordering/hs2/hs2_pacemaker.h"

#include <algorithm>

namespace resdb {
namespace hs2 {

Hs2Pacemaker::Hs2Pacemaker(const ResDBConfig& config,
                           SystemInfo* system_info)
    : system_info_(system_info), current_view_(1) {
  for (const auto& replica : config.GetReplicaInfos()) {
    if (replica.id() > 0) {
      replica_ids_.push_back(replica.id());
    }
  }
  std::sort(replica_ids_.begin(), replica_ids_.end());
  if (system_info_ != nullptr && system_info_->GetCurrentView() > 0) {
    current_view_ = system_info_->GetCurrentView();
  }
  const uint32_t leader = CurrentLeader();
  if (system_info_ != nullptr && leader != 0) {
    system_info_->SetPrimary(leader);
    system_info_->SetCurrentView(current_view_);
  }
}

uint64_t Hs2Pacemaker::CurrentView() const { return current_view_; }

uint32_t Hs2Pacemaker::CurrentLeader() const {
  return LeaderForView(current_view_);
}

uint32_t Hs2Pacemaker::LeaderForView(uint64_t view) const {
  if (replica_ids_.empty() || view == 0) {
    return 0;
  }
  const size_t index = static_cast<size_t>((view - 1) % replica_ids_.size());
  return replica_ids_[index];
}

bool Hs2Pacemaker::AdvanceView(uint64_t new_view) {
  if (new_view <= current_view_) {
    return false;
  }
  const uint32_t leader = LeaderForView(new_view);
  if (leader == 0) {
    return false;
  }
  current_view_ = new_view;
  if (system_info_ != nullptr) {
    system_info_->SetPrimary(leader);
    system_info_->SetCurrentView(new_view);
  }
  return true;
}

}  // namespace hs2
}  // namespace resdb
