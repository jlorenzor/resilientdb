#include "platform/consensus/ordering/hs2/hs2_consensus.h"

#include <map>
#include <set>
#include <sstream>

namespace resdb {
namespace hs2 {

Hs2Consensus::Hs2Consensus(int replica_count)
    : replica_count_(replica_count),
      quorum_size_((2 * ((replica_count - 1) / 3)) + 1) {}

int Hs2Consensus::ReplicaCount() const { return replica_count_; }

int Hs2Consensus::QuorumSize() const { return quorum_size_; }

bool Hs2Consensus::CanVote(const Hs2Block& block) const {
  if (block.height <= 0 || block.view <= 0) {
    return false;
  }
  if (block.block_hash.empty() || block.payload_digest.empty()) {
    return false;
  }
  if (block.proposer_id <= 0 || block.proposer_id > replica_count_) {
    return false;
  }
  return true;
}

bool Hs2Consensus::IsValidQuorumCertificate(
    const Hs2QuorumCertificate& qc) const {
  if (qc.height <= 0 || qc.view <= 0 || qc.block_hash.empty()) {
    return false;
  }
  if (static_cast<int>(qc.voters.size()) < quorum_size_) {
    return false;
  }
  std::set<int> unique_voters;
  for (int voter : qc.voters) {
    if (voter <= 0 || voter > replica_count_) {
      return false;
    }
    unique_voters.insert(voter);
  }
  return static_cast<int>(unique_voters.size()) >= quorum_size_;
}

bool Hs2Consensus::WouldCommit(const Hs2QuorumCertificate& qc) const {
  return qc.phase == Hs2Phase::kPhase2 && IsValidQuorumCertificate(qc);
}

bool Hs2Consensus::RecordVote(const Hs2Vote& vote) {
  if (vote.replica_id <= 0 || vote.replica_id > replica_count_) {
    return false;
  }
  if (vote.height <= 0 || vote.view <= 0 || vote.block_hash.empty()) {
    return false;
  }
  const std::string key = VoteKey(vote);
  auto it = votes_by_slot_.find(key);
  if (it != votes_by_slot_.end() && it->second.block_hash != vote.block_hash) {
    return false;
  }
  votes_by_slot_[key] = vote;
  return true;
}

void Hs2Consensus::SetLockedQc(const Hs2QuorumCertificate& qc) {
  if (IsValidQuorumCertificate(qc)) {
    locked_qc_ = qc;
  }
}

bool Hs2Consensus::IsSafeProposal(
    const Hs2Block& block, const Hs2QuorumCertificate& justify_qc) const {
  if (!CanVote(block) || !IsValidQuorumCertificate(justify_qc)) {
    return false;
  }
  if (locked_qc_.block_hash.empty()) {
    return true;
  }
  if (block.parent_hash == locked_qc_.block_hash) {
    return true;
  }
  return justify_qc.view > locked_qc_.view;
}

std::string Hs2Consensus::VoteKey(const Hs2Vote& vote) const {
  std::ostringstream key;
  key << vote.replica_id << ":" << vote.height << ":" << vote.view << ":"
      << static_cast<int>(vote.phase);
  return key.str();
}

}  // namespace hs2
}  // namespace resdb
