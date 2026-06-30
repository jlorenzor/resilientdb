#include "platform/consensus/ordering/hs2/hs2_qc_boundary.h"

#include <set>
#include <sstream>

#include "common/crypto/signature_verifier.h"

namespace resdb {
namespace hs2 {

namespace {

int PhaseNumber(Hs2Phase phase) { return static_cast<int>(phase); }

}  // namespace

Hs2VoteVerifier::Hs2VoteVerifier(int replica_count)
    : replica_count_(replica_count) {}

bool Hs2VoteVerifier::VerifyVote(const Hs2Vote& vote, const Hs2Block& block,
                                 Hs2Phase phase, std::string* reason) const {
  if (vote.replica_id <= 0 || vote.replica_id > replica_count_) {
    if (reason != nullptr) *reason = "vote replica id out of range";
    return false;
  }
  if (vote.height != block.height || vote.view != block.view) {
    if (reason != nullptr) *reason = "vote height/view mismatch";
    return false;
  }
  if (vote.block_hash != block.block_hash) {
    if (reason != nullptr) *reason = "vote block hash mismatch";
    return false;
  }
  if (vote.phase != phase) {
    if (reason != nullptr) *reason = "vote phase mismatch";
    return false;
  }
  if (vote.signature.empty()) {
    if (reason != nullptr) *reason = "vote signature missing";
    return false;
  }
  if (vote.signature.find("hs2-alpha-vote|") != 0) {
    if (reason != nullptr) *reason = "vote signature outside alpha boundary";
    return false;
  }
  return true;
}

Hs2VoteCollector::Hs2VoteCollector(int quorum_size,
                                   const Hs2VoteVerifier* verifier)
    : quorum_size_(quorum_size), verifier_(verifier) {}

bool Hs2VoteCollector::AddVote(const Hs2Vote& vote, const Hs2Block& block,
                               Hs2Phase phase, std::string* reason) {
  if (verifier_ == nullptr ||
      !verifier_->VerifyVote(vote, block, phase, reason)) {
    return false;
  }
  const auto it = votes_by_replica_.find(vote.replica_id);
  if (it != votes_by_replica_.end()) {
    if (it->second.block_hash == vote.block_hash && it->second.phase == phase) {
      return true;
    }
    if (reason != nullptr) *reason = "conflicting vote for replica";
    return false;
  }
  votes_by_replica_[vote.replica_id] = vote;
  return true;
}

bool Hs2VoteCollector::HasQuorum() const {
  return static_cast<int>(votes_by_replica_.size()) >= quorum_size_;
}

std::vector<Hs2Vote> Hs2VoteCollector::Votes() const {
  std::vector<Hs2Vote> votes;
  for (const auto& entry : votes_by_replica_) {
    votes.push_back(entry.second);
  }
  return votes;
}

Hs2QcBuilder::Hs2QcBuilder(int quorum_size) : quorum_size_(quorum_size) {}

Hs2QuorumCertificate Hs2QcBuilder::Build(
    const Hs2Block& block, Hs2Phase phase,
    const std::vector<Hs2Vote>& votes) const {
  return BuildForParent(block.height, block.view, block.block_hash, phase,
                        votes);
}

Hs2QuorumCertificate Hs2QcBuilder::BuildForParent(
    int64_t height, int64_t view, const std::string& block_hash, Hs2Phase phase,
    const std::vector<Hs2Vote>& votes) const {
  Hs2QuorumCertificate qc;
  qc.height = height;
  qc.view = view;
  qc.block_hash = block_hash;
  qc.phase = phase;
  std::set<int> unique_voters;
  for (const auto& vote : votes) {
    if (!unique_voters.insert(vote.replica_id).second) {
      continue;
    }
    qc.voters.push_back(vote.replica_id);
    qc.vote_signatures.push_back(vote.signature);
  }
  qc.proof_digest = ProofDigest(qc);
  return qc;
}

bool Hs2QcBuilder::Validate(const Hs2QuorumCertificate& qc,
                            const Hs2Block& block, Hs2Phase phase,
                            std::string* reason) const {
  if (qc.height != block.height || qc.view != block.view ||
      qc.block_hash != block.block_hash) {
    if (reason != nullptr) *reason = "qc does not match block";
    return false;
  }
  if (qc.phase != phase) {
    if (reason != nullptr) *reason = "qc phase mismatch";
    return false;
  }
  if (static_cast<int>(qc.voters.size()) < quorum_size_) {
    if (reason != nullptr) *reason = "qc quorum too small";
    return false;
  }
  if (qc.voters.size() != qc.vote_signatures.size()) {
    if (reason != nullptr) *reason = "qc signature count mismatch";
    return false;
  }
  if (qc.proof_digest.empty()) {
    if (reason != nullptr) *reason = "qc proof digest missing";
    return false;
  }
  return true;
}

std::string Hs2QcBuilder::ProofDigest(const Hs2QuorumCertificate& qc) const {
  std::ostringstream input;
  input << "hs2:qc:" << qc.height << ":" << qc.view << ":"
        << qc.block_hash << ":" << PhaseNumber(qc.phase);
  for (int voter : qc.voters) {
    input << ":voter=" << voter;
  }
  for (const auto& signature : qc.vote_signatures) {
    input << ":sig=" << signature;
  }
  return SignatureVerifier::CalculateHash(input.str());
}

Hs2LocalVoteSource::Hs2LocalVoteSource(int replica_count, int quorum_size)
    : replica_count_(replica_count), quorum_size_(quorum_size) {}

std::vector<int> Hs2LocalVoteSource::QuorumVoters() const {
  std::vector<int> voters;
  for (int id = 1; id <= replica_count_ &&
                   static_cast<int>(voters.size()) < quorum_size_;
       ++id) {
    voters.push_back(id);
  }
  return voters;
}

std::vector<Hs2Vote> Hs2LocalVoteSource::CreateVotes(
    const Hs2Block& block, Hs2Phase phase) const {
  return CreateParentVotes(block.height, block.view, block.block_hash, phase);
}

std::vector<Hs2Vote> Hs2LocalVoteSource::CreateParentVotes(
    int64_t height, int64_t view, const std::string& block_hash,
    Hs2Phase phase) const {
  std::vector<Hs2Vote> votes;
  for (int voter : QuorumVoters()) {
    Hs2Vote vote;
    vote.replica_id = voter;
    vote.height = height;
    vote.view = view;
    vote.block_hash = block_hash;
    vote.phase = phase;
    vote.signature = AlphaSignature(voter, height, view, block_hash, phase);
    votes.push_back(vote);
  }
  return votes;
}

std::string Hs2LocalVoteSource::AlphaSignature(
    int replica_id, int64_t height, int64_t view,
    const std::string& block_hash, Hs2Phase phase) const {
  std::ostringstream signature;
  signature << "hs2-alpha-vote|replica=" << replica_id
            << "|height=" << height << "|view=" << view
            << "|phase=" << PhaseNumber(phase) << "|block=" << block_hash;
  return signature.str();
}

}  // namespace hs2
}  // namespace resdb
