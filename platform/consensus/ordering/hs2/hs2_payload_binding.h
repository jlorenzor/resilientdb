#pragma once

#include <string>

#include "platform/consensus/ordering/hs2/hs2_types.h"
#include "platform/proto/resdb.pb.h"

namespace resdb {
namespace hs2 {

struct Hs2CommitBinding {
  std::string payload_digest;
  std::string request_hash;
  std::string block_digest;
  std::string phase1_proof_digest;
  std::string phase2_proof_digest;
  std::string execution_digest;
  std::string binding_digest;
};

class Hs2CommitProofBuilder {
 public:
  Hs2CommitBinding BuildBinding(
      const Request& request, const Hs2Block& block,
      const Hs2QuorumCertificate& phase1_qc,
      const Hs2QuorumCertificate& phase2_qc) const;
  std::string Serialize(const Request& request, const Hs2Block& block,
                        const Hs2QuorumCertificate& phase1_qc,
                        const Hs2QuorumCertificate& phase2_qc) const;
  bool VerifyForRequest(const Request& request,
                        const std::string& serialized_proof,
                        std::string* reason) const;

 private:
  std::string PayloadDigest(const Request& request) const;
  std::string RequestHash(const Request& request) const;
  std::string ExecutionDigest(const Hs2CommitBinding& binding) const;
  std::string BindingDigest(const Hs2CommitBinding& binding) const;
  std::string StableDigest(const std::string& input) const;
  std::string HexEncode(const std::string& bytes) const;
  std::string FindField(const std::string& serialized_proof,
                        const std::string& name) const;
};

}  // namespace hs2
}  // namespace resdb
