#include <iostream>

#include "platform/consensus/ordering/hs2/hs2_consensus.h"

int main() {
  resdb::hs2::Hs2Consensus consensus(/*replica_count=*/4);
  resdb::hs2::Hs2Block block;
  block.height = 1;
  block.view = 1;
  block.block_hash = "block-1";
  block.parent_hash = "genesis";
  block.payload_digest = "sha256:test";
  block.proposer_id = 1;

  resdb::hs2::Hs2QuorumCertificate qc;
  qc.height = 1;
  qc.view = 1;
  qc.block_hash = "block-1";
  qc.phase = resdb::hs2::Hs2Phase::kPhase2;
  qc.voters = {1, 2, 3};

  if (!consensus.IsValidQuorumCertificate(qc)) {
    std::cerr << "invalid qc" << std::endl;
    return 1;
  }
  if (!consensus.CanVote(block)) {
    std::cerr << "unsafe block" << std::endl;
    return 2;
  }
  std::cout << "hs2 skeleton ok" << std::endl;
  return 0;
}
