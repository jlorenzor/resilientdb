#pragma once

#include <cstdint>
#include <vector>

#include "platform/config/resdb_config.h"
#include "platform/consensus/execution/system_info.h"

namespace resdb {
namespace hs2 {

// Minimal deterministic pacemaker for the v2.9.11 leader-recovery track.
// It updates SystemInfo, but it does not yet implement timeout certificates,
// highQC transfer, or signed NEWVIEW messages.
class Hs2Pacemaker {
 public:
  Hs2Pacemaker(const ResDBConfig& config, SystemInfo* system_info);

  uint64_t CurrentView() const;
  uint32_t CurrentLeader() const;
  uint32_t LeaderForView(uint64_t view) const;
  bool AdvanceView(uint64_t new_view);

 private:
  SystemInfo* system_info_;
  uint64_t current_view_;
  std::vector<uint32_t> replica_ids_;
};

}  // namespace hs2
}  // namespace resdb
