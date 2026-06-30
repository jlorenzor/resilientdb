#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "platform/config/resdb_config.h"
#include "platform/consensus/ordering/hs2/hs2_consensus.h"
#include "platform/consensus/ordering/hs2/hs2_qc_boundary.h"
#include "platform/proto/resdb.pb.h"

namespace resdb {
namespace hs2 {

struct Hs2NewTxnCertification {
  bool committed = false;
  Hs2Block block;
  Hs2QuorumCertificate phase1_qc;
  Hs2QuorumCertificate phase2_qc;
  std::string reason;
};

// Hs2NewTxnPipeline is an experimental v2.17.1 gate for TYPE_NEW_TXNS.
// It does not claim production-grade networked HS2 yet. Its purpose is to stop
// the KV path from committing blindly through MessageManagerBasic: a request
// must first pass the HS2 block/vote/QC/commit rules modeled by Hs2Consensus.
class Hs2NewTxnPipeline {
 public:
  explicit Hs2NewTxnPipeline(const ResDBConfig& config);

  Hs2NewTxnCertification Certify(Request* request, uint32_t leader_id,
                                 uint64_t view);

 private:
  Hs2Block BuildBlock(const Request& request, uint32_t leader_id,
                      uint64_t view) const;
  Hs2QuorumCertificate BuildJustifyQc(const Hs2Block& block) const;
  Hs2QuorumCertificate BuildPhaseQc(const Hs2Block& block, Hs2Phase phase);
  Hs2NewTxnCertification CertifyAlreadyCommitted(const Request& request,
                                                  uint32_t leader_id,
                                                  uint64_t view) const;
  void AttachCommitProof(Request* request,
                         const Hs2NewTxnCertification& certification) const;
  std::string PayloadDigest(const Request& request) const;
  std::string BlockHash(const Hs2Block& block) const;
  std::string ParentHash(const Request& request) const;
  std::string SerializeCommitProof(
      const Hs2NewTxnCertification& certification) const;

 private:
  int replica_count_;
  int quorum_size_;
  Hs2Consensus consensus_;
  Hs2LocalVoteSource local_vote_source_;
  Hs2VoteVerifier vote_verifier_;
  Hs2QcBuilder qc_builder_;
  mutable std::mutex mutex_;
  std::string last_committed_block_hash_;
  int64_t last_committed_height_;
  int64_t last_committed_view_;
};

}  // namespace hs2
}  // namespace resdb
