#pragma once

#include <map>
#include <string>

#include "platform/consensus/ordering/hs2/hs2_types.h"

namespace resdb {
namespace hs2 {

class Hs2Consensus {
 public:
  explicit Hs2Consensus(int replica_count);

  int ReplicaCount() const;
  int QuorumSize() const;

  bool CanVote(const Hs2Block& block) const;
  bool IsValidQuorumCertificate(const Hs2QuorumCertificate& qc) const;
  bool WouldCommit(const Hs2QuorumCertificate& qc) const;
  bool RecordVote(const Hs2Vote& vote);
  void SetLockedQc(const Hs2QuorumCertificate& qc);
  bool IsSafeProposal(const Hs2Block& block,
                      const Hs2QuorumCertificate& justify_qc) const;

 private:
  int replica_count_;
  int quorum_size_;
  Hs2QuorumCertificate locked_qc_;
  std::map<std::string, Hs2Vote> votes_by_slot_;

  std::string VoteKey(const Hs2Vote& vote) const;
};

}  // namespace hs2
}  // namespace resdb
