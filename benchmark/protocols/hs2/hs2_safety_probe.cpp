#include <iostream>

#include "platform/consensus/ordering/hs2/hs2_consensus.h"

namespace {

int Fail(const char* message) {
  std::cerr << message << std::endl;
  return 1;
}

}  // namespace

int main() {
  resdb::hs2::Hs2Consensus consensus(/*replica_count=*/4);

  resdb::hs2::Hs2QuorumCertificate valid_qc;
  valid_qc.height = 1;
  valid_qc.view = 1;
  valid_qc.block_hash = "block-1";
  valid_qc.phase = resdb::hs2::Hs2Phase::kPhase2;
  valid_qc.voters = {1, 2, 3};
  if (!consensus.IsValidQuorumCertificate(valid_qc)) {
    return Fail("valid qc rejected");
  }

  resdb::hs2::Hs2QuorumCertificate duplicate_qc = valid_qc;
  duplicate_qc.voters = {1, 1, 2};
  if (consensus.IsValidQuorumCertificate(duplicate_qc)) {
    return Fail("duplicate voter qc accepted");
  }

  resdb::hs2::Hs2Block unsafe_block;
  unsafe_block.height = 1;
  unsafe_block.view = 1;
  unsafe_block.block_hash = "bad";
  unsafe_block.payload_digest = "sha256:bad";
  unsafe_block.proposer_id = 9;
  if (consensus.CanVote(unsafe_block)) {
    return Fail("invalid proposer accepted");
  }

  resdb::hs2::Hs2Vote vote;
  vote.replica_id = 1;
  vote.height = 2;
  vote.view = 2;
  vote.block_hash = "block-2a";
  vote.phase = resdb::hs2::Hs2Phase::kPhase1;
  if (!consensus.RecordVote(vote)) {
    return Fail("first vote rejected");
  }

  resdb::hs2::Hs2Vote conflict = vote;
  conflict.block_hash = "block-2b";
  if (consensus.RecordVote(conflict)) {
    return Fail("conflicting vote accepted");
  }

  consensus.SetLockedQc(valid_qc);
  resdb::hs2::Hs2Block child;
  child.height = 2;
  child.view = 2;
  child.block_hash = "block-2";
  child.parent_hash = "block-1";
  child.payload_digest = "sha256:child";
  child.proposer_id = 2;
  if (!consensus.IsSafeProposal(child, valid_qc)) {
    return Fail("safe child rejected");
  }

  resdb::hs2::Hs2Block conflicting_child = child;
  conflicting_child.parent_hash = "other-parent";
  if (consensus.IsSafeProposal(conflicting_child, valid_qc)) {
    return Fail("conflicting child accepted under lock");
  }

  std::cout << "hs2 safety ok" << std::endl;
  return 0;
}
