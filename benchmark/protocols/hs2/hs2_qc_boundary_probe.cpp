#include <iostream>
#include <string>

#include "platform/consensus/ordering/hs2/hs2_qc_boundary.h"

namespace {

int Fail(const char* message) {
  std::cerr << message << std::endl;
  return 1;
}

resdb::hs2::Hs2Block MakeBlock() {
  resdb::hs2::Hs2Block block;
  block.height = 11;
  block.view = 4;
  block.parent_hash = "parent-hash";
  block.payload_digest = "payload-digest";
  block.proposer_id = 2;
  block.block_hash = "block-hash";
  return block;
}

}  // namespace

int main() {
  const auto block = MakeBlock();
  resdb::hs2::Hs2LocalVoteSource source(/*replica_count=*/4,
                                        /*quorum_size=*/3);
  resdb::hs2::Hs2VoteVerifier verifier(/*replica_count=*/4);
  resdb::hs2::Hs2VoteCollector collector(/*quorum_size=*/3, &verifier);
  const auto votes =
      source.CreateVotes(block, resdb::hs2::Hs2Phase::kPhase1);

  if (votes.size() != 3) {
    return Fail("unexpected local quorum vote count");
  }

  std::string reason;
  if (!collector.AddVote(votes[0], block, resdb::hs2::Hs2Phase::kPhase1,
                         &reason) ||
      !collector.AddVote(votes[1], block, resdb::hs2::Hs2Phase::kPhase1,
                         &reason)) {
    return Fail("valid votes rejected before quorum");
  }
  if (collector.HasQuorum()) {
    return Fail("collector reached quorum too early");
  }
  if (!collector.AddVote(votes[2], block, resdb::hs2::Hs2Phase::kPhase1,
                         &reason) ||
      !collector.HasQuorum()) {
    return Fail("collector did not reach quorum");
  }
  if (!collector.AddVote(votes[2], block, resdb::hs2::Hs2Phase::kPhase1,
                         &reason)) {
    return Fail("duplicate identical vote should be idempotent");
  }

  auto conflicting_vote = votes[0];
  conflicting_vote.block_hash = "conflicting-block";
  if (collector.AddVote(conflicting_vote, block,
                        resdb::hs2::Hs2Phase::kPhase1, &reason)) {
    return Fail("conflicting vote was accepted");
  }

  auto unsigned_vote = votes[0];
  unsigned_vote.signature.clear();
  if (verifier.VerifyVote(unsigned_vote, block,
                          resdb::hs2::Hs2Phase::kPhase1, &reason)) {
    return Fail("unsigned vote was accepted");
  }

  resdb::hs2::Hs2QcBuilder builder(/*quorum_size=*/3);
  const auto qc = builder.Build(block, resdb::hs2::Hs2Phase::kPhase1,
                                collector.Votes());
  if (qc.voters.size() != 3 || qc.vote_signatures.size() != 3 ||
      qc.proof_digest.empty()) {
    return Fail("qc did not carry explicit vote/proof material");
  }
  if (!builder.Validate(qc, block, resdb::hs2::Hs2Phase::kPhase1, &reason)) {
    return Fail("built qc was not valid");
  }

  std::cout << "hs2 qc boundary ok" << std::endl;
  return 0;
}
