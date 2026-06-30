#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace resdb {
namespace hs2 {

enum class Hs2Phase {
  kPhase1 = 1,
  kPhase2 = 2,
};

struct Hs2Block {
  int64_t height = 0;
  int64_t view = 0;
  std::string block_hash;
  std::string parent_hash;
  std::string payload_digest;
  int proposer_id = 0;
};

struct Hs2Vote {
  int replica_id = 0;
  int64_t height = 0;
  int64_t view = 0;
  std::string block_hash;
  Hs2Phase phase = Hs2Phase::kPhase1;
  std::string signature;
};

struct Hs2QuorumCertificate {
  int64_t height = 0;
  int64_t view = 0;
  std::string block_hash;
  Hs2Phase phase = Hs2Phase::kPhase1;
  std::vector<int> voters;
};

}  // namespace hs2
}  // namespace resdb
