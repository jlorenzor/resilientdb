#include "platform/consensus/ordering/hs2/consensus_manager_hs2.h"

#include <utility>

#include <glog/logging.h>

#include "platform/proto/resdb.pb.h"

namespace resdb {
namespace hs2 {

namespace {

constexpr int kNotImplementedYet = -100;
constexpr int kUnsupportedRequestType = -101;

}  // namespace

ConsensusManagerHs2::ConsensusManagerHs2(
    const ResDBConfig& config, std::unique_ptr<TransactionManager> executor,
    std::unique_ptr<CustomQuery> query_executor)
    : ConsensusManager(config),
      transaction_manager_(executor.get()),
      query_executor_(std::move(query_executor)),
      system_info_(std::make_unique<SystemInfo>(config)),
      pacemaker_(std::make_unique<Hs2Pacemaker>(config, system_info_.get())),
      message_manager_(std::make_unique<MessageManagerBasic>(
          config, std::move(executor), system_info_.get())),
      commitment_(std::make_unique<CommitmentBasic>(
          config_, message_manager_.get(), GetBroadCastClient(),
          GetSignatureVerifier())),
      response_manager_(std::make_unique<common::ResponseManager>(
          config_, GetBroadCastClient(), system_info_.get(),
          GetSignatureVerifier())),
      new_txn_pipeline_(std::make_unique<Hs2NewTxnPipeline>(config_)),
      core_(static_cast<int>(config.GetReplicaNum())),
      leader_id_(config.GetReplicaInfos().empty()
                     ? 0
                     : config.GetReplicaInfos().front().id()),
      current_view_(1),
      next_seq_(1) {}

const char* ConsensusManagerHs2::ProtocolName() const { return "hs2"; }

const Hs2Consensus& ConsensusManagerHs2::Core() const { return core_; }

std::vector<ReplicaInfo> ConsensusManagerHs2::GetReplicas() {
  return config_.GetReplicaInfos();
}

uint32_t ConsensusManagerHs2::GetPrimary() { return leader_id_; }

uint32_t ConsensusManagerHs2::GetVersion() { return current_view_; }

void ConsensusManagerHs2::SetPrimary(uint32_t primary, uint64_t version) {
  if (version < current_view_) {
    return;
  }
  leader_id_ = primary;
  current_view_ = static_cast<uint32_t>(version);
  if (system_info_ != nullptr) {
    system_info_->SetPrimary(primary);
    system_info_->SetCurrentView(version);
  }
}

void ConsensusManagerHs2::Start() { ConsensusManager::Start(); }

bool ConsensusManagerHs2::SupportsRequestType(Request::Type type) const {
  switch (type) {
    case Request::TYPE_CLIENT_REQUEST:
    case Request::TYPE_RESPONSE:
    case Request::TYPE_NEW_TXNS:
    case Request::TYPE_CUSTOM_QUERY:
    case Request::TYPE_CUSTOM_CONSENSUS:
      return true;
    default:
      return false;
  }
}

int ConsensusManagerHs2::ConsensusCommit(std::unique_ptr<Context> context,
                                         std::unique_ptr<Request> request) {
  if (request == nullptr ||
      !SupportsRequestType(static_cast<Request::Type>(request->type()))) {
    return kUnsupportedRequestType;
  }

  switch (request->type()) {
    case Request::TYPE_CLIENT_REQUEST:
      return HandleClientRequest(std::move(context), std::move(request));
    case Request::TYPE_RESPONSE:
      return response_manager_->ProcessResponseMsg(std::move(context),
                                                   std::move(request));
    case Request::TYPE_NEW_TXNS:
      return HandleNewTransactions(std::move(context), std::move(request));
    case Request::TYPE_CUSTOM_QUERY:
      return HandleCustomQuery(std::move(context), std::move(request));
    case Request::TYPE_CUSTOM_CONSENSUS:
      return HandleConsensusMessage(std::move(context), std::move(request));
    default:
      return kUnsupportedRequestType;
  }
}

int ConsensusManagerHs2::HandleClientRequest(
    std::unique_ptr<Context> context, std::unique_ptr<Request> request) {
  std::vector<uint64_t> fallback_views;
  if (pacemaker_ != nullptr) {
    fallback_views.push_back(pacemaker_->CurrentView() + 1);
  }
  return response_manager_->NewUserRequestWithFallbackViews(
      std::move(context), std::move(request), fallback_views);
}

int ConsensusManagerHs2::HandleCustomQuery(
    std::unique_ptr<Context> context, std::unique_ptr<Request> request) {
  (void)context;
  (void)request;
  if (query_executor_ == nullptr) {
    return kNotImplementedYet;
  }
  return kNotImplementedYet;
}

int ConsensusManagerHs2::HandleConsensusMessage(
    std::unique_ptr<Context> context, std::unique_ptr<Request> request) {
  (void)context;
  (void)request;
  return kNotImplementedYet;
}

int ConsensusManagerHs2::HandleNewTransactions(
    std::unique_ptr<Context> context, std::unique_ptr<Request> request) {
  (void)context;
  if (request == nullptr) {
    return kUnsupportedRequestType;
  }
  if (message_manager_ == nullptr) {
    return kNotImplementedYet;
  }

  const uint32_t self_id = config_.GetSelfInfo().id();
  if (request->seq() == 0) {
    request->set_seq(next_seq_.fetch_add(1));
  }
  if (request->current_view() == 0) {
    request->set_current_view(current_view_);
  }

  if (request->current_view() > current_view_ && pacemaker_ != nullptr &&
      pacemaker_->AdvanceView(request->current_view())) {
    leader_id_ = pacemaker_->CurrentLeader();
    current_view_ = static_cast<uint32_t>(pacemaker_->CurrentView());
  }

  const bool is_leader = self_id == leader_id_;
  const bool came_from_proxy =
      static_cast<uint32_t>(request->sender_id()) != leader_id_;

  if (is_leader && came_from_proxy) {
    auto replica_request = std::make_unique<Request>(*request);
    replica_request->set_sender_id(self_id);
    BroadCast(*replica_request);
    request->set_sender_id(self_id);
  } else if (request->sender_id() == 0) {
    request->set_sender_id(self_id);
  }

  const auto certification =
      new_txn_pipeline_->Certify(request.get(), leader_id_, current_view_);
  if (!certification.committed) {
    LOG(ERROR) << "CHATAY_HS2_PIPELINE rejected"
               << " seq=" << request->seq()
               << " view=" << request->current_view()
               << " leader=" << leader_id_
               << " reason=" << certification.reason;
    return kNotImplementedYet;
  }

  LOG(INFO) << "CHATAY_HS2_PIPELINE certified"
            << " seq=" << request->seq()
            << " view=" << request->current_view()
            << " leader=" << leader_id_
            << " block=" << certification.block.block_hash
            << " phase2_voters=" << certification.phase2_qc.voters.size();

  return message_manager_->Commit(std::move(request));
}

TransactionManager* ConsensusManagerHs2::GetTransactionManager() const {
  return transaction_manager_;
}

CustomQuery* ConsensusManagerHs2::GetCustomQueryExecutor() const {
  return query_executor_.get();
}

}  // namespace hs2
}  // namespace resdb
