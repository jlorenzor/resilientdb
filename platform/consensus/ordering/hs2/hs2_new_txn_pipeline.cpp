#include "platform/consensus/ordering/hs2/hs2_new_txn_pipeline.h"

#include <algorithm>
#include <sstream>

#include "common/crypto/signature_verifier.h"

namespace resdb {
namespace hs2 {

namespace {

int CalculateQuorumSize(int replica_count) {
  if (replica_count <= 0) {
    return 0;
  }
  const int f = (replica_count - 1) / 3;
  return (2 * f) + 1;
}

}  // namespace

Hs2NewTxnPipeline::Hs2NewTxnPipeline(const ResDBConfig& config)
    : replica_count_(static_cast<int>(config.GetReplicaNum())),
      quorum_size_(CalculateQuorumSize(replica_count_)),
      consensus_(replica_count_),
      local_vote_source_(replica_count_, quorum_size_),
      vote_verifier_(replica_count_),
      qc_builder_(quorum_size_),
      last_committed_block_hash_("hs2-genesis"),
      last_committed_height_(0),
      last_committed_view_(0) {}

Hs2NewTxnCertification Hs2NewTxnPipeline::Certify(Request* request,
                                                  uint32_t leader_id,
                                                  uint64_t view) {
  std::lock_guard<std::mutex> lock(mutex_);
  Hs2NewTxnCertification result;
  if (request == nullptr) {
    result.reason = "request is null";
    return result;
  }
  if (leader_id == 0 || static_cast<int>(leader_id) > replica_count_) {
    result.reason = "invalid leader";
    return result;
  }
  if (view == 0) {
    result.reason = "invalid view";
    return result;
  }
  if (request->seq() > 0 &&
      static_cast<int64_t>(request->seq()) <= last_committed_height_) {
    result = CertifyAlreadyCommitted(*request, leader_id, view);
    AttachCommitProof(request, result);
    return result;
  }

  result.block = BuildBlock(*request, leader_id, view);
  if (!consensus_.CanVote(result.block)) {
    result.reason = "block failed CanVote";
    return result;
  }

  const auto justify_qc = BuildJustifyQc(result.block);
  if (!consensus_.IsSafeProposal(result.block, justify_qc)) {
    result.reason = "proposal failed safety rule";
    return result;
  }

  result.phase1_qc = BuildPhaseQc(result.block, Hs2Phase::kPhase1);
  if (!consensus_.IsValidQuorumCertificate(result.phase1_qc)) {
    result.reason = "phase1 qc invalid";
    return result;
  }
  consensus_.SetLockedQc(result.phase1_qc);

  result.phase2_qc = BuildPhaseQc(result.block, Hs2Phase::kPhase2);
  if (!consensus_.WouldCommit(result.phase2_qc)) {
    result.reason = "phase2 qc does not commit";
    return result;
  }

  result.committed = true;
  result.reason = "committed by experimental hs2 phase2 qc";
  last_committed_block_hash_ = result.block.block_hash;
  last_committed_height_ = result.block.height;
  last_committed_view_ = result.block.view;
  AttachCommitProof(request, result);
  return result;
}

Hs2Block Hs2NewTxnPipeline::BuildBlock(const Request& request,
                                       uint32_t leader_id,
                                       uint64_t view) const {
  Hs2Block block;
  block.height = request.seq() > 0 ? static_cast<int64_t>(request.seq()) : 1;
  block.view = static_cast<int64_t>(view);
  block.parent_hash = ParentHash(request);
  block.payload_digest = PayloadDigest(request);
  block.proposer_id = static_cast<int>(leader_id);
  block.block_hash = BlockHash(block);
  return block;
}

Hs2QuorumCertificate Hs2NewTxnPipeline::BuildJustifyQc(
    const Hs2Block& block) const {
  Hs2QuorumCertificate qc;
  qc.height = last_committed_height_ > 0 ? last_committed_height_ : 1;
  qc.view = last_committed_view_ > 0 ? last_committed_view_ : block.view;
  qc.block_hash = block.parent_hash.empty() ? "hs2-genesis" : block.parent_hash;
  qc.phase = Hs2Phase::kPhase2;
  qc = qc_builder_.BuildForParent(
      qc.height, qc.view, qc.block_hash, qc.phase,
      local_vote_source_.CreateParentVotes(qc.height, qc.view, qc.block_hash,
                                           qc.phase));
  return qc;
}

Hs2QuorumCertificate Hs2NewTxnPipeline::BuildPhaseQc(const Hs2Block& block,
                                                     Hs2Phase phase) {
  Hs2VoteCollector collector(quorum_size_, &vote_verifier_);
  for (const auto& vote : local_vote_source_.CreateVotes(block, phase)) {
    std::string reason;
    if (!collector.AddVote(vote, block, phase, &reason)) {
      continue;
    }
    if (!consensus_.RecordVote(vote)) {
      continue;
    }
  }
  if (!collector.HasQuorum()) {
    return Hs2QuorumCertificate{};
  }
  return qc_builder_.Build(block, phase, collector.Votes());
}

Hs2NewTxnCertification Hs2NewTxnPipeline::CertifyAlreadyCommitted(
    const Request& request, uint32_t leader_id, uint64_t view) const {
  Hs2NewTxnCertification result;
  result.committed = true;
  result.reason = "already committed by experimental hs2 pipeline";
  result.block.height =
      request.seq() > 0 ? static_cast<int64_t>(request.seq()) : 1;
  result.block.view = static_cast<int64_t>(view);
  result.block.parent_hash = last_committed_block_hash_;
  result.block.payload_digest = PayloadDigest(request);
  result.block.proposer_id = static_cast<int>(leader_id);
  result.block.block_hash = last_committed_block_hash_;

  result.phase1_qc.height = result.block.height;
  result.phase1_qc.view = result.block.view;
  result.phase1_qc.block_hash = last_committed_block_hash_;
  result.phase1_qc.phase = Hs2Phase::kPhase1;
  result.phase1_qc = qc_builder_.BuildForParent(
      result.block.height, result.block.view, result.block.block_hash,
      Hs2Phase::kPhase1,
      local_vote_source_.CreateParentVotes(result.block.height,
                                           result.block.view,
                                           result.block.block_hash,
                                           Hs2Phase::kPhase1));

  result.phase2_qc.height = result.block.height;
  result.phase2_qc.view = result.block.view;
  result.phase2_qc.block_hash = last_committed_block_hash_;
  result.phase2_qc.phase = Hs2Phase::kPhase2;
  result.phase2_qc = qc_builder_.BuildForParent(
      result.block.height, result.block.view, result.block.block_hash,
      Hs2Phase::kPhase2,
      local_vote_source_.CreateParentVotes(result.block.height,
                                           result.block.view,
                                           result.block.block_hash,
                                           Hs2Phase::kPhase2));
  return result;
}

void Hs2NewTxnPipeline::AttachCommitProof(
    Request* request, const Hs2NewTxnCertification& certification) const {
  if (request == nullptr || !certification.committed) {
    return;
  }
  SignatureInfo* proof = request->mutable_committed_certs()->add_committed_certs();
  proof->set_hash_type(SignatureInfo::NONE);
  proof->set_node_id(certification.block.proposer_id);
  proof->set_signature(commit_proof_builder_.Serialize(
      *request, certification.block, certification.phase1_qc,
      certification.phase2_qc));
}

std::string Hs2NewTxnPipeline::PayloadDigest(const Request& request) const {
  if (!request.hash().empty()) {
    return request.hash();
  }
  if (!request.data_hash().empty()) {
    return request.data_hash();
  }
  return SignatureVerifier::CalculateHash(request.data());
}

std::string Hs2NewTxnPipeline::BlockHash(const Hs2Block& block) const {
  std::ostringstream input;
  input << "hs2:block:" << block.height << ":" << block.view << ":"
        << block.parent_hash << ":" << block.payload_digest << ":"
        << block.proposer_id;
  return SignatureVerifier::CalculateHash(input.str());
}

std::string Hs2NewTxnPipeline::ParentHash(const Request& request) const {
  if (request.seq() <= 1 || last_committed_block_hash_.empty()) {
    return "hs2-genesis";
  }
  return last_committed_block_hash_;
}

}  // namespace hs2
}  // namespace resdb
