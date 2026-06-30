#pragma once

#include <map>
#include <string>
#include <vector>

#include "platform/consensus/ordering/hs2/hs2_types.h"

namespace resdb {
namespace hs2 {

// v2.17.2 separates the alpha/local vote model from QC construction. The
// verifier is intentionally not a production cryptographic verifier yet; it is
// the explicit boundary where real signature checks must be attached.
class Hs2VoteVerifier {
 public:
  explicit Hs2VoteVerifier(int replica_count);

  bool VerifyVote(const Hs2Vote& vote, const Hs2Block& block, Hs2Phase phase,
                  std::string* reason) const;

 private:
  int replica_count_;
};

class Hs2VoteCollector {
 public:
  Hs2VoteCollector(int quorum_size, const Hs2VoteVerifier* verifier);

  bool AddVote(const Hs2Vote& vote, const Hs2Block& block, Hs2Phase phase,
               std::string* reason);
  bool HasQuorum() const;
  std::vector<Hs2Vote> Votes() const;

 private:
  int quorum_size_;
  const Hs2VoteVerifier* verifier_;
  std::map<int, Hs2Vote> votes_by_replica_;
};

class Hs2QcBuilder {
 public:
  explicit Hs2QcBuilder(int quorum_size);

  Hs2QuorumCertificate Build(const Hs2Block& block, Hs2Phase phase,
                             const std::vector<Hs2Vote>& votes) const;
  Hs2QuorumCertificate BuildForParent(int64_t height, int64_t view,
                                      const std::string& block_hash,
                                      Hs2Phase phase,
                                      const std::vector<Hs2Vote>& votes) const;
  bool Validate(const Hs2QuorumCertificate& qc, const Hs2Block& block,
                Hs2Phase phase, std::string* reason) const;

 private:
  std::string ProofDigest(const Hs2QuorumCertificate& qc) const;

 private:
  int quorum_size_;
};

class Hs2LocalVoteSource {
 public:
  Hs2LocalVoteSource(int replica_count, int quorum_size);

  std::vector<int> QuorumVoters() const;
  std::vector<Hs2Vote> CreateVotes(const Hs2Block& block,
                                   Hs2Phase phase) const;
  std::vector<Hs2Vote> CreateParentVotes(int64_t height, int64_t view,
                                         const std::string& block_hash,
                                         Hs2Phase phase) const;

 private:
  std::string AlphaSignature(int replica_id, int64_t height, int64_t view,
                             const std::string& block_hash,
                             Hs2Phase phase) const;

 private:
  int replica_count_;
  int quorum_size_;
};

}  // namespace hs2
}  // namespace resdb
