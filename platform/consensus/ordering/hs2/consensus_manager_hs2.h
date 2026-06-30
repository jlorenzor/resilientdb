#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include "executor/common/custom_query.h"
#include "executor/common/transaction_manager.h"
#include "platform/config/resdb_config.h"
#include "platform/consensus/execution/system_info.h"
#include "platform/consensus/ordering/common/commitment_basic.h"
#include "platform/consensus/ordering/common/message_manager_basic.h"
#include "platform/consensus/ordering/common/response_manager.h"
#include "platform/consensus/ordering/hs2/hs2_consensus.h"
#include "platform/consensus/ordering/hs2/hs2_pacemaker.h"
#include "platform/networkstrate/consensus_manager.h"
#include "platform/proto/resdb.pb.h"

namespace resdb {
namespace hs2 {

// ConsensusManagerHs2 is the first C++ wiring layer between ResilientDB's
// ServiceNetwork/ConsensusManager surface and the HS2 core rules. It is not
// a complete HotStuff-2 implementation yet: the v2.9.7 runtime path executes
// KV batches through ResilientDB's common executor, while network QC
// aggregation, pacemaker, persistence, and fault handling remain milestones.
class ConsensusManagerHs2 : public ConsensusManager {
 public:
  ConsensusManagerHs2(
      const ResDBConfig& config,
      std::unique_ptr<TransactionManager> executor = nullptr,
      std::unique_ptr<CustomQuery> query_executor = nullptr);
  ~ConsensusManagerHs2() override = default;

  int ConsensusCommit(std::unique_ptr<Context> context,
                      std::unique_ptr<Request> request) override;

  std::vector<ReplicaInfo> GetReplicas() override;
  uint32_t GetPrimary() override;
  uint32_t GetVersion() override;
  void SetPrimary(uint32_t primary, uint64_t version) override;
  void Start() override;

  const char* ProtocolName() const;
  const Hs2Consensus& Core() const;
  bool SupportsRequestType(Request::Type type) const;

  TransactionManager* GetTransactionManager() const;
  CustomQuery* GetCustomQueryExecutor() const;

 private:
  int HandleClientRequest(std::unique_ptr<Context> context,
                          std::unique_ptr<Request> request);
  int HandleCustomQuery(std::unique_ptr<Context> context,
                        std::unique_ptr<Request> request);
  int HandleConsensusMessage(std::unique_ptr<Context> context,
                             std::unique_ptr<Request> request);
  int HandleNewTransactions(std::unique_ptr<Context> context,
                            std::unique_ptr<Request> request);

 private:
  TransactionManager* transaction_manager_;
  std::unique_ptr<CustomQuery> query_executor_;
  std::unique_ptr<SystemInfo> system_info_;
  std::unique_ptr<Hs2Pacemaker> pacemaker_;
  std::unique_ptr<MessageManagerBasic> message_manager_;
  std::unique_ptr<CommitmentBasic> commitment_;
  std::unique_ptr<common::ResponseManager> response_manager_;
  Hs2Consensus core_;
  uint32_t leader_id_;
  uint32_t current_view_;
  std::atomic<uint64_t> next_seq_;
};

}  // namespace hs2
}  // namespace resdb
