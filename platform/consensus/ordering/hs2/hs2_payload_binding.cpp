#include "platform/consensus/ordering/hs2/hs2_payload_binding.h"

#include <iomanip>
#include <sstream>

#include "common/crypto/signature_verifier.h"

namespace resdb {
namespace hs2 {

Hs2CommitBinding Hs2CommitProofBuilder::BuildBinding(
    const Request& request, const Hs2Block& block,
    const Hs2QuorumCertificate& phase1_qc,
    const Hs2QuorumCertificate& phase2_qc) const {
  Hs2CommitBinding binding;
  binding.payload_digest = PayloadDigest(request);
  binding.request_hash = RequestHash(request);
  binding.block_digest = StableDigest(block.block_hash);
  binding.phase1_proof_digest =
      phase1_qc.proof_digest.empty() ? StableDigest("missing-phase1-proof")
                                     : StableDigest(phase1_qc.proof_digest);
  binding.phase2_proof_digest =
      phase2_qc.proof_digest.empty() ? StableDigest("missing-phase2-proof")
                                     : StableDigest(phase2_qc.proof_digest);
  binding.execution_digest = ExecutionDigest(binding);
  binding.binding_digest = BindingDigest(binding);
  return binding;
}

std::string Hs2CommitProofBuilder::Serialize(
    const Request& request, const Hs2Block& block,
    const Hs2QuorumCertificate& phase1_qc,
    const Hs2QuorumCertificate& phase2_qc) const {
  const Hs2CommitBinding binding =
      BuildBinding(request, block, phase1_qc, phase2_qc);
  std::ostringstream proof;
  proof << "hs2-alpha-commit"
        << "|height=" << block.height
        << "|view=" << block.view
        << "|block_digest=" << binding.block_digest
        << "|payload_digest=" << binding.payload_digest
        << "|request_hash=" << binding.request_hash
        << "|phase1_proof=" << binding.phase1_proof_digest
        << "|phase2_proof=" << binding.phase2_proof_digest
        << "|execution_digest=" << binding.execution_digest
        << "|binding_digest=" << binding.binding_digest
        << "|phase1_voters=";
  for (size_t i = 0; i < phase1_qc.voters.size(); ++i) {
    if (i > 0) proof << ",";
    proof << phase1_qc.voters[i];
  }
  proof << "|phase2_voters=";
  for (size_t i = 0; i < phase2_qc.voters.size(); ++i) {
    if (i > 0) proof << ",";
    proof << phase2_qc.voters[i];
  }
  return proof.str();
}

bool Hs2CommitProofBuilder::VerifyForRequest(
    const Request& request, const std::string& serialized_proof,
    std::string* reason) const {
  if (serialized_proof.find("hs2-alpha-commit|") != 0) {
    if (reason != nullptr) *reason = "missing hs2 alpha commit marker";
    return false;
  }
  const std::string payload_digest =
      FindField(serialized_proof, "payload_digest");
  const std::string request_hash = FindField(serialized_proof, "request_hash");
  const std::string block_digest = FindField(serialized_proof, "block_digest");
  const std::string phase1_proof = FindField(serialized_proof, "phase1_proof");
  const std::string phase2_proof = FindField(serialized_proof, "phase2_proof");
  const std::string execution_digest =
      FindField(serialized_proof, "execution_digest");
  const std::string binding_digest =
      FindField(serialized_proof, "binding_digest");

  if (payload_digest.empty() || request_hash.empty() || block_digest.empty() ||
      phase1_proof.empty() || phase2_proof.empty() ||
      execution_digest.empty() || binding_digest.empty()) {
    if (reason != nullptr) *reason = "commit proof missing binding fields";
    return false;
  }
  if (payload_digest != PayloadDigest(request)) {
    if (reason != nullptr) *reason = "payload digest mismatch";
    return false;
  }
  if (request_hash != RequestHash(request)) {
    if (reason != nullptr) *reason = "request hash mismatch";
    return false;
  }

  Hs2CommitBinding binding;
  binding.payload_digest = payload_digest;
  binding.request_hash = request_hash;
  binding.block_digest = block_digest;
  binding.phase1_proof_digest = phase1_proof;
  binding.phase2_proof_digest = phase2_proof;
  binding.execution_digest = ExecutionDigest(binding);
  if (binding.execution_digest != execution_digest) {
    if (reason != nullptr) *reason = "execution digest mismatch";
    return false;
  }
  binding.binding_digest = BindingDigest(binding);
  if (binding.binding_digest != binding_digest) {
    if (reason != nullptr) *reason = "binding digest mismatch";
    return false;
  }
  return true;
}

std::string Hs2CommitProofBuilder::PayloadDigest(
    const Request& request) const {
  if (!request.hash().empty()) {
    return StableDigest(request.hash());
  }
  if (!request.data_hash().empty()) {
    return StableDigest(request.data_hash());
  }
  return StableDigest(request.data());
}

std::string Hs2CommitProofBuilder::RequestHash(const Request& request) const {
  std::ostringstream input;
  input << "hs2:request:"
        << request.type() << ":" << request.seq() << ":"
        << request.current_view() << ":" << request.sender_id() << ":"
        << request.proxy_id() << ":" << request.data() << ":"
        << request.hash() << ":" << request.data_hash();
  return StableDigest(input.str());
}

std::string Hs2CommitProofBuilder::ExecutionDigest(
    const Hs2CommitBinding& binding) const {
  std::ostringstream input;
  input << "hs2:execute:" << binding.block_digest << ":"
        << binding.request_hash << ":" << binding.payload_digest << ":"
        << binding.phase2_proof_digest;
  return StableDigest(input.str());
}

std::string Hs2CommitProofBuilder::BindingDigest(
    const Hs2CommitBinding& binding) const {
  std::ostringstream input;
  input << "hs2:binding:" << binding.payload_digest << ":"
        << binding.request_hash << ":" << binding.block_digest << ":"
        << binding.phase1_proof_digest << ":"
        << binding.phase2_proof_digest << ":"
        << binding.execution_digest;
  return StableDigest(input.str());
}

std::string Hs2CommitProofBuilder::StableDigest(
    const std::string& input) const {
  return HexEncode(SignatureVerifier::CalculateHash(input));
}

std::string Hs2CommitProofBuilder::HexEncode(const std::string& bytes) const {
  std::ostringstream encoded;
  encoded << std::hex << std::setfill('0');
  for (unsigned char byte : bytes) {
    encoded << std::setw(2) << static_cast<int>(byte);
  }
  return encoded.str();
}

std::string Hs2CommitProofBuilder::FindField(
    const std::string& serialized_proof, const std::string& name) const {
  const std::string needle = "|" + name + "=";
  const size_t start = serialized_proof.find(needle);
  if (start == std::string::npos) {
    return "";
  }
  const size_t value_start = start + needle.size();
  const size_t value_end = serialized_proof.find('|', value_start);
  if (value_end == std::string::npos) {
    return serialized_proof.substr(value_start);
  }
  return serialized_proof.substr(value_start, value_end - value_start);
}

}  // namespace hs2
}  // namespace resdb
