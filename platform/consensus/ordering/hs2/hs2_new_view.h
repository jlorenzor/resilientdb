#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "platform/consensus/ordering/hs2/hs2_consensus.h"
#include "platform/consensus/ordering/hs2/hs2_timeout_certificate.h"

namespace resdb {
namespace hs2 {

struct Hs2NewViewMessage {
  int replica_id = 0;
  int64_t new_view = 0;
  Hs2TimeoutCertificate timeout_certificate;
  Hs2QuorumCertificate high_qc;
  std::string signature;
};

enum class Hs2NewViewRecordStatus {
  kAccepted = 0,
  kDuplicateReplica = 1,
  kInvalidReplica = 2,
  kInvalidView = 3,
  kWrongView = 4,
  kInvalidTimeoutCertificate = 5,
  kInvalidHighQc = 6,
};

// Collects NEW_VIEW messages for one target view. Each message must carry a
// valid timeout certificate for the previous view and may carry the sender's
// local highQC. Once 2f + 1 distinct replicas are observed, the collector
// exposes the highest safe highQC for the new leader.
class Hs2NewViewCollector {
 public:
  Hs2NewViewCollector(int replica_count, int64_t new_view);

  int ReplicaCount() const;
  int QuorumSize() const;
  int64_t NewView() const;
  bool HasQuorum() const;
  bool HasSelectedHighQc() const;
  const Hs2QuorumCertificate& SelectedHighQc() const;
  std::vector<int> Voters() const;

  Hs2NewViewRecordStatus RecordNewView(const Hs2NewViewMessage& message);
  bool IsSafeProposal(const Hs2Block& block) const;

 private:
  Hs2NewViewRecordStatus ValidateMessage(
      const Hs2NewViewMessage& message) const;

 private:
  int replica_count_;
  int quorum_size_;
  int64_t new_view_;
  bool quorum_formed_ = false;
  Hs2Consensus consensus_;
  std::map<int, Hs2NewViewMessage> messages_by_replica_;
  Hs2QuorumCertificate selected_high_qc_;
};

const char* NewViewRecordStatusName(Hs2NewViewRecordStatus status);

}  // namespace hs2
}  // namespace resdb
