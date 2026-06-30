#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace resdb {
namespace hs2 {

struct Hs2TimeoutMessage {
  int replica_id = 0;
  int64_t view = 0;
  int64_t high_qc_view = 0;
  std::string high_qc_block_hash;
  std::string signature;
};

struct Hs2TimeoutCertificate {
  int64_t view = 0;
  int64_t high_qc_view = 0;
  std::string high_qc_block_hash;
  std::vector<int> voters;
  std::vector<Hs2TimeoutMessage> timeouts;
};

enum class Hs2TimeoutRecordStatus {
  kAccepted = 0,
  kDuplicateReplica = 1,
  kInvalidReplica = 2,
  kInvalidView = 3,
  kWrongView = 4,
  kInvalidHighQc = 5,
};

// Collects timeout votes for one HS2 view and forms a timeout certificate once
// the collector observes 2f + 1 distinct replicas. This is intentionally a
// small component: it validates quorum, duplicate voters, view consistency, and
// highQC metadata before later patches wire certificates into new-view.
class Hs2TimeoutCollector {
 public:
  Hs2TimeoutCollector(int replica_count, int64_t target_view);

  int ReplicaCount() const;
  int QuorumSize() const;
  int64_t TargetView() const;
  bool HasCertificate() const;
  const Hs2TimeoutCertificate& Certificate() const;
  std::vector<int> Voters() const;

  Hs2TimeoutRecordStatus RecordTimeout(const Hs2TimeoutMessage& timeout);
  bool IsValidCertificate(const Hs2TimeoutCertificate& certificate) const;

 private:
  Hs2TimeoutRecordStatus ValidateTimeout(
      const Hs2TimeoutMessage& timeout) const;
  void FormCertificate();

 private:
  int replica_count_;
  int quorum_size_;
  int64_t target_view_;
  bool certificate_formed_ = false;
  std::map<int, Hs2TimeoutMessage> timeouts_by_replica_;
  Hs2TimeoutCertificate certificate_;
};

const char* TimeoutRecordStatusName(Hs2TimeoutRecordStatus status);

}  // namespace hs2
}  // namespace resdb
