/*
 * Copyright (c) 2019-2022 ExpoLab, UC Davis
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "platform/consensus/ordering/hotstuff/commitment.h"

#include <glog/logging.h>
#include <unistd.h>

#include <chrono>
#include <thread>

#include "common/utils/utils.h"

namespace resdb {
namespace hotstuff {

namespace {

int64_t ColdStartNowMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

}  // namespace

Commitment::Commitment(const ResDBConfig& config,
                       MessageManager* message_manager,
                       ReplicaCommunicator* replica_communicator,
                       SignatureVerifier* verifier)
    : CommitmentBasic(config, message_manager, replica_communicator, verifier),
      message_manager_(message_manager) {
  current_view_ = 0;
}

Commitment::~Commitment() {}

void Commitment::Init() {
  const int64_t started_at = ColdStartNowMs();
  LOG(ERROR) << "CHATAY_HS1_COLD_START commitment_init_enter"
             << " ts_ms=" << started_at
             << " self=" << id_;
  current_view_ = 0;
  LOG(ERROR) << "CHATAY_HS1_TRACE commitment_init"
             << " self=" << id_
             << " current_view=" << current_view_
             << " primary_next=" << PrimaryId(current_view_ + 1)
             << " quorum=" << config_.GetMinDataReceiveNum();
  // Chatay HS1 v2.11.3-beta.2:
  // PR100 can emit NEWVIEW before all local key material / replica channels
  // are usable in a cold local bootstrap. This delay only widens bootstrap
  // readiness; it does not change quorum, locking, QC, or vote validation.
  const int bootstrap_wait_seconds = 2 + id_;
  LOG(ERROR) << "CHATAY_HS1_COLD_START newview_bootstrap_wait_start"
             << " ts_ms=" << ColdStartNowMs()
             << " self=" << id_
             << " seconds=" << bootstrap_wait_seconds;
  LOG(ERROR) << "CHATAY_HS1_TRACE newview_bootstrap_wait self=" << id_
             << " seconds=" << bootstrap_wait_seconds;
  sleep(bootstrap_wait_seconds);
  LOG(ERROR) << "CHATAY_HS1_COLD_START newview_bootstrap_wait_finish"
             << " ts_ms=" << ColdStartNowMs()
             << " duration_ms=" << (ColdStartNowMs() - started_at)
             << " self=" << id_;
  LOG(ERROR) << "CHATAY_HS1_COLD_START send_newview_initial_start"
             << " ts_ms=" << ColdStartNowMs()
             << " self=" << id_;
  SendNewView();
  LOG(ERROR) << "CHATAY_HS1_COLD_START commitment_init_exit"
             << " ts_ms=" << ColdStartNowMs()
             << " duration_ms=" << (ColdStartNowMs() - started_at)
             << " self=" << id_
             << " current_view=" << current_view_;
}

QC Commitment::GetQC(int64_t view_num, HotStuffRequest::Type type) {
  QC qc;
  qc.set_type(type);
  for (const auto& request : received_requests_[view_num % 128][type]) {
    *qc.mutable_node() = request->node();
    *qc.add_signatures() = request->node_signature();
  }
  return qc;
}

QC Commitment::GetHighQC(int64_t view_num) {
  QC high_qc;
  high_qc.set_type(HotStuffRequest::TYPE_PRECOMMIT);
  for (const auto& request :
       received_requests_[view_num % 128][HotStuffRequest::TYPE_PREPARE_VOTE]) {
    if (request->node().info().view() > high_qc.node().info().view()) {
      *high_qc.mutable_node() = request->node();
    }
    *high_qc.add_signatures() = request->node_signature();
  }
  return high_qc;
}

HotStuffRequest::Type Commitment::GetNextState(int type) {
  switch (type) {
    case HotStuffRequest::TYPE_PREPARE:
    case HotStuffRequest::TYPE_PREPARE_VOTE:
      return HotStuffRequest::TYPE_PRECOMMIT;
    case HotStuffRequest::TYPE_PRECOMMIT:
    case HotStuffRequest::TYPE_PRECOMMIT_VOTE:
      return HotStuffRequest::TYPE_COMMIT;
    case HotStuffRequest::TYPE_COMMIT:
    case HotStuffRequest::TYPE_COMMIT_VOTE:
      return HotStuffRequest::TYPE_DECIDE;
    case HotStuffRequest::TYPE_DECIDE:
      return HotStuffRequest::TYPE_NEWVIEW;
    case HotStuffRequest::TYPE_NONE:
      return HotStuffRequest::TYPE_NEWVIEW;
    default:
      return HotStuffRequest::TYPE_NONE;
  }
}

HotStuffRequest::Type Commitment::VoteType(int type) {
  switch (type) {
    case HotStuffRequest::TYPE_PREPARE:
      return HotStuffRequest::TYPE_PREPARE_VOTE;
    case HotStuffRequest::TYPE_PRECOMMIT:
      return HotStuffRequest::TYPE_PRECOMMIT_VOTE;
    case HotStuffRequest::TYPE_COMMIT:
      return HotStuffRequest::TYPE_COMMIT_VOTE;
    default:
      return HotStuffRequest::TYPE_NONE;
  }
}

std::unique_ptr<Request> Commitment::NewRequest(const HotStuffRequest& request,
                                                HotStuffRequest::Type type) {
  HotStuffRequest new_request(request);
  if (type != HotStuffRequest::TYPE_NONE) {
    new_request.set_type(type);
  } else {
    HotStuffRequest::Type next_type = GetNextState(request.type());
    new_request.set_type(next_type);
  }
  new_request.set_sender_id(id_);
  auto ret = std::make_unique<Request>();
  new_request.SerializeToString(ret->mutable_data());
  return ret;
}

class Timer {
 public:
  Timer(std::string name) {
    start_ = GetCurrentTime();
    name_ = name;
  }
  ~Timer() {
    // LOG(ERROR) << name_ << " run time:" << (GetCurrentTime() - start_);
  }

 private:
  uint64_t start_;
  std::string name_;
};

// 1. Client sends requests to the primary. Primary broadcasts a prepare
// message.
// 2. each replica responses a prepare vote.
// 3. Primary receives 2f+1 votes for prepare, sends out a pre-commit message.
int Commitment::Process(std::unique_ptr<HotStuffRequest> request) {
  Timer timer(HotStuffRequest_Type_Name(request->type()));
  LOG(ERROR) << "CHATAY_HS1_TRACE commitment_process"
             << " self=" << id_
             << " type=" << HotStuffRequest_Type_Name(request->type())
             << " request_view=" << request->view()
             << " node_view=" << request->node().info().view()
             << " qc_view=" << request->qc().node().info().view()
             << " sender=" << request->sender_id()
             << " current_view=" << current_view_;
  switch (request->type()) {
    case HotStuffRequest::TYPE_NEWREQUEST:
      return ProcessNewRequest(std::move(request));
    case HotStuffRequest::TYPE_PREPARE_VOTE:
    case HotStuffRequest::TYPE_PRECOMMIT_VOTE:
    case HotStuffRequest::TYPE_COMMIT_VOTE:
    case HotStuffRequest::TYPE_NEWVIEW: {
      return ProcessMessageOnPrimary(std::move(request));
    }
    case HotStuffRequest::TYPE_PREPARE:
    case HotStuffRequest::TYPE_PRECOMMIT:
    case HotStuffRequest::TYPE_COMMIT:
    case HotStuffRequest::TYPE_DECIDE: {
      std::unique_ptr<HotStuffRequest> user_request = nullptr;
      if (request->type() == HotStuffRequest::TYPE_PRECOMMIT) {
        if (request->has_new_user_request()) {
          user_request =
              std::make_unique<HotStuffRequest>(request->new_user_request());
        }
      }
      ProcessMessageOnReplica(std::move(request));
      if (user_request) {
        ProcessMessageOnReplica(std::move(user_request));
      }
      return 0;
    }
    default:
      return -2;
  }
}

int Commitment::ProcessNewRequest(std::unique_ptr<HotStuffRequest> request) {
  request->mutable_node()->mutable_info()->set_hash(
      SignatureVerifier::CalculateHash(request->node().data()));
  global_stats_->IncClientRequest();
  const bool queue_empty_before = request_list_.Empty();
  LOG(ERROR) << "CHATAY_HS1_TRACE new_request_queued"
             << " self=" << id_
             << " request_view=" << request->view()
             << " node_view=" << request->node().info().view()
             << " current_view=" << current_view_
             << " primary_current=" << PrimaryId(current_view_)
             << " queue_empty_before=" << queue_empty_before
             << " proxy=" << request->node().proxy_id()
             << " data_size=" << request->node().data().size()
             << " hash=" << request->node().info().hash();
  request_list_.Push(std::move(request));
  LOG(ERROR) << "CHATAY_HS1_TRACE new_request_push_done"
             << " self=" << id_
             << " current_view=" << current_view_
             << " queue_empty_after=" << request_list_.Empty();
  return 0;
}

std::unique_ptr<HotStuffRequest> Commitment::GetClientRequest() {
  int64_t wait_loops = 0;
  const int64_t wait_ms = config_.ClientBatchWaitTimeMS();
  while (!stop_) {
    const bool queue_empty_before = request_list_.Empty();
    auto request = request_list_.Pop(wait_ms);
    if (request == nullptr) {
      wait_loops++;
      if (wait_loops == 1 || wait_loops % 50 == 0) {
        LOG(ERROR) << "CHATAY_HS1_TRACE client_request_wait_timeout"
                   << " self=" << id_
                   << " current_view=" << current_view_
                   << " wait_ms=" << wait_ms
                   << " wait_loops=" << wait_loops
                   << " queue_empty_before=" << queue_empty_before
                   << " queue_empty_after=" << request_list_.Empty();
      }
      continue;
    }
    LOG(ERROR) << "CHATAY_HS1_TRACE client_request_dequeued"
               << " self=" << id_
               << " request_view=" << request->view()
               << " node_view=" << request->node().info().view()
               << " current_view=" << current_view_
               << " wait_loops=" << wait_loops
               << " queue_empty_after=" << request_list_.Empty()
               << " proxy=" << request->node().proxy_id()
               << " data_size=" << request->node().data().size();
    return request;
  }
  return nullptr;
}

// ========= Start function ===============
void Commitment::SendNewView() {
  const int64_t started_at = ColdStartNowMs();
  LOG(ERROR) << "CHATAY_HS1_COLD_START send_newview_enter"
             << " ts_ms=" << started_at
             << " self=" << id_
             << " current_view_before=" << current_view_;
  QC prepare_qc = message_manager_->GetPrepareQC();
  auto user_request = std::make_unique<HotStuffRequest>();
  *user_request->mutable_qc() = prepare_qc;
  user_request->set_view(current_view_);
  std::unique_ptr<Request> vote_request =
      NewRequest(*user_request, HotStuffRequest::TYPE_NEWVIEW);
  current_view_++;
  // LOG(ERROR) << "send new msg view:" << current_view_<<" to
  // primary:"<<PrimaryId(current_view_); send vote to the primary.
  LOG(ERROR) << "CHATAY_HS1_TRACE send_newview"
             << " self=" << id_
             << " request_view=" << user_request->view()
             << " current_view_after_increment=" << current_view_
             << " target_primary=" << PrimaryId(current_view_)
             << " prepare_qc_view=" << prepare_qc.node().info().view()
             << " prepare_qc_sigs=" << prepare_qc.signatures_size();
  for (int attempt = 1; attempt <= 3; ++attempt) {
    if (attempt > 1) {
      sleep(3);
    }
    LOG(ERROR) << "CHATAY_HS1_TRACE send_newview_attempt"
               << " self=" << id_
               << " attempt=" << attempt
               << " target_primary=" << PrimaryId(current_view_);
    LOG(ERROR) << "CHATAY_HS1_COLD_START send_newview_attempt"
               << " ts_ms=" << ColdStartNowMs()
               << " self=" << id_
               << " attempt=" << attempt
               << " target_primary=" << PrimaryId(current_view_)
               << " current_view=" << current_view_;
    replica_communicator_->SendMessage(*vote_request, PrimaryId(current_view_));
  }
  LOG(ERROR) << "CHATAY_HS1_COLD_START send_newview_exit"
             << " ts_ms=" << ColdStartNowMs()
             << " duration_ms=" << (ColdStartNowMs() - started_at)
             << " self=" << id_
             << " current_view_after=" << current_view_;
}

// 1. Obtain the highQC from 2f+1 new view messages.
// 2. Create a leave on the hightQC.
// 3. Broadcast a prepare message.
int Commitment::ProcessNewView(int64_t view_number) {
  LOG(ERROR) << "CHATAY_HS1_TRACE process_newview_enter"
             << " self=" << id_
             << " view=" << view_number
             << " expected_primary=" << PrimaryId(view_number)
             << " current_view=" << current_view_;
  if (PrimaryId(view_number) != id_) {
    LOG(ERROR) << "not current primary:" << view_number;
    return -2;
  }
  LOG(ERROR) << "CHATAY_HS1_TRACE process_newview_waiting_request"
             << " self=" << id_
             << " view=" << view_number
             << " current_view=" << current_view_
             << " queue_empty_before=" << request_list_.Empty();
  auto user_request = GetClientRequest();
  if (user_request == nullptr) {
    LOG(ERROR) << "CHATAY_HS1_TRACE process_newview_no_request"
               << " self=" << id_
               << " view=" << view_number
               << " current_view=" << current_view_
               << " queue_empty_after=" << request_list_.Empty();
    LOG(ERROR) << "data is empty";
    return -2;
  }
  QC high_qc = GetHighQC(view_number - 1);
  QC prepare_qc = message_manager_->GetPrepareQC();
  LOG(ERROR) << "CHATAY_HS1_TRACE process_newview_highqc_candidate"
             << " self=" << id_
             << " view=" << view_number
             << " high_qc_view=" << high_qc.node().info().view()
             << " high_qc_sigs=" << high_qc.signatures_size()
             << " prepare_qc_view=" << prepare_qc.node().info().view()
             << " prepare_qc_sigs=" << prepare_qc.signatures_size();
  if (prepare_qc.node().info().view() > high_qc.node().info().view() ||
      (high_qc.signatures_size() == 0 && prepare_qc.signatures_size() > 0)) {
    LOG(ERROR) << "CHATAY_HS1_TRACE process_newview_highqc_fallback_prepareqc"
               << " self=" << id_
               << " view=" << view_number
               << " from_high_qc_view=" << high_qc.node().info().view()
               << " to_prepare_qc_view=" << prepare_qc.node().info().view();
    high_qc = prepare_qc;
  }
  LOG(ERROR) << "CHATAY_HS1_TRACE process_newview_prepare"
             << " self=" << id_
             << " view=" << view_number
             << " high_qc_view=" << high_qc.node().info().view()
             << " high_qc_sigs=" << high_qc.signatures_size()
             << " user_data_size=" << user_request->node().data().size()
             << " proxy=" << user_request->node().proxy_id();
  // LOG(ERROR) << "receive new view. view number:" << view_number;
  // LOG(ERROR) << "high qc:" << high_qc.node().info().view()<<"
  // hash:"<<high_qc.node().info().hash();
  *user_request->mutable_node()->mutable_pre() = high_qc.node().info();
  user_request->mutable_node()->mutable_info()->set_view(view_number);
  *user_request->mutable_qc() = high_qc;

  std::unique_ptr<Request> new_request =
      NewRequest(*user_request, HotStuffRequest::TYPE_PREPARE);
  current_view_ = view_number;
  // LOG(ERROR) << "new view start:" << current_view_;
  LOG(ERROR) << "CHATAY_HS1_TRACE broadcast_prepare"
             << " self=" << id_
             << " view=" << view_number
             << " request_type=TYPE_PREPARE"
             << " data_size=" << new_request->data().size();
  replica_communicator_->BroadCast(*new_request);
  return 0;
}

std::unique_ptr<HotStuffRequest> Commitment::GetNewViewMessage(
    int64_t view_number) {
  if (PrimaryId(view_number) != id_) {
    LOG(ERROR) << "not current primary:" << view_number;
    return nullptr;
  }
  QC high_qc = GetHighQC(view_number - 1);
  auto user_request = GetClientRequest();
  if (user_request == nullptr) {
    LOG(ERROR) << "data is empty";
    return nullptr;
  }
  LOG(ERROR) << "CHATAY_HS1_TRACE get_newview_message"
             << " self=" << id_
             << " view=" << view_number
             << " high_qc_view=" << high_qc.node().info().view()
             << " high_qc_sigs=" << high_qc.signatures_size()
             << " user_data_size=" << user_request->node().data().size();
  *user_request->mutable_node()->mutable_pre() = high_qc.node().info();
  user_request->mutable_node()->mutable_info()->set_view(view_number);
  user_request->set_view(view_number);
  *user_request->mutable_qc() = high_qc;
  user_request->set_type(HotStuffRequest::TYPE_PREPARE);
  return user_request;
}

bool Commitment::VerifyNodeSignagure(const HotStuffRequest& request) {
  if (verifier_ == nullptr) {
    return true;
  }

  if (!request.has_node_signature()) {
    LOG(ERROR) << "node signature is empty";
    return false;
  }
  std::string data;
  request.node().SerializeToString(&data);
  // check signatures
  bool valid = verifier_->VerifyMessage(data, request.node_signature());
  if (!valid) {
    LOG(ERROR) << "signature is not valid:"
               << request.node_signature().DebugString();
    return false;
  }
  return true;
}

bool Commitment::VerifyTS(const HotStuffRequest& request) {
  if (verifier_ == nullptr) {
    return true;
  }
  if (request.qc().signatures_size() < config_.GetMinDataReceiveNum()) {
    LOG(ERROR) << "signature is not enough:" << request.qc().signatures_size();
    return false;
  }
  std::string data;
  request.qc().node().SerializeToString(&data);
  for (const auto& signature : request.qc().signatures()) {
    bool valid = verifier_->VerifyMessage(data, signature);
    if (!valid) {
      LOG(ERROR) << "TS is not valid";
      return false;
    }
  }
  return true;
}

int Commitment::ProcessMessageOnPrimary(
    std::unique_ptr<HotStuffRequest> request) {
  HotStuffRequest::Type type = (HotStuffRequest::Type)request->type();
  LOG(ERROR) << "CHATAY_HS1_TRACE primary_enter"
             << " self=" << id_
             << " type=" << HotStuffRequest_Type_Name(type)
             << " request_view=" << request->view()
             << " node_view=" << request->node().info().view()
             << " qc_view=" << request->qc().node().info().view()
             << " sender=" << request->sender_id()
             << " current_view=" << current_view_;
  // LOG(INFO) << "primary get type:" << HotStuffRequest_Type_Name(type)
  //           << " view:" << request->node().info().view()
  //           << " from:" << request->sender_id();
  if (type != HotStuffRequest::TYPE_NEWVIEW) {
    if (!VerifyNodeSignagure(*request)) {
      LOG(ERROR) << "CHATAY_HS1_TRACE primary_reject_node_signature"
                 << " self=" << id_
                 << " type=" << HotStuffRequest_Type_Name(type)
                 << " sender=" << request->sender_id()
                 << " request_view=" << request->view();
      LOG(ERROR) << "signature invalid";
      return -2;
    }
  } else {
    if (PrimaryId(request->view() + 1) != id_) {
      LOG(ERROR) << "CHATAY_HS1_TRACE primary_reject_wrong_newview_primary"
                 << " self=" << id_
                 << " sender=" << request->sender_id()
                 << " request_view=" << request->view()
                 << " expected_primary=" << PrimaryId(request->view() + 1);
      LOG(ERROR) << "not current primary:" << request->view() + 1;
      return -2;
    }
  }

  int64_t view_num = request->view();
  int64_t node_view = request->node().info().view();
  int64_t qc_view = request->qc().node().info().view();
  int32_t sender_id = request->sender_id();
  int receive_size = 0;
  if (view_num < current_view_ - 5) {
    LOG(ERROR) << "CHATAY_HS1_TRACE primary_reject_old_view"
               << " self=" << id_
               << " type=" << HotStuffRequest_Type_Name(type)
               << " view=" << view_num
               << " current_view=" << current_view_;
    LOG(ERROR) << " view :" << view_num
               << " is too old, current view:" << current_view_;
    return -2;
  }
  {
    std::unique_lock<std::mutex> lk(mutex_[view_num % 128]);
    auto ret =
        received_senders_[view_num % 128][type].insert(request->sender_id());
    if (!ret.second) {
      LOG(ERROR) << "CHATAY_HS1_TRACE primary_duplicate_sender"
                 << " self=" << id_
                 << " type=" << HotStuffRequest_Type_Name(type)
                 << " sender=" << request->sender_id()
                 << " view=" << request->view();
      LOG(ERROR) << "sender:" << request->sender_id()
                 << " has been received. view:" << request->view();
      return -2;
    }
    received_requests_[view_num % 128][type].push_back(std::move(request));
    receive_size = received_senders_[view_num % 128][type].size();
  }

  {
    std::unique_lock<std::mutex> lk(mutex_[128]);
    if (type == HotStuffRequest::TYPE_PREPARE_VOTE) {
      current_view_ = std::max(current_view_, node_view);
    }
  }

  // LOG(ERROR) << "primary get size:" << receive_size
  //            << " type:" << HotStuffRequest_Type_Name(type)
  //            << " view:" << view_num << " qc view:" << qc_view
  //            << " from:" << sender_id << " node view:" << node_view;
  // if have received 2f+1 votes, broadcast next message.
  LOG(ERROR) << "CHATAY_HS1_TRACE primary_receive_count"
             << " self=" << id_
             << " type=" << HotStuffRequest_Type_Name(type)
             << " view=" << view_num
             << " sender=" << sender_id
             << " node_view=" << node_view
             << " qc_view=" << qc_view
             << " receive_size=" << receive_size
             << " quorum=" << config_.GetMinDataReceiveNum();
  if (receive_size == config_.GetMinDataReceiveNum()) {
    LOG(ERROR) << "CHATAY_HS1_TRACE primary_quorum_reached"
               << " self=" << id_
               << " type=" << HotStuffRequest_Type_Name(type)
               << " view=" << view_num
               << " receive_size=" << receive_size;
    if (type == HotStuffRequest::TYPE_NEWVIEW) {
      // Only for the first view to boost up the server.
      return ProcessNewView(view_num + 1);
    }
    // New node has been added in QC.
    HotStuffRequest new_hotstuff_request;
    new_hotstuff_request.set_type(type);
    new_hotstuff_request.set_view(view_num);
    *new_hotstuff_request.mutable_qc() = GetQC(view_num, type);

    if (type == HotStuffRequest::TYPE_PREPARE_VOTE &&
        view_num == current_view_) {
      std::unique_ptr<Request> new_request = NewRequest(new_hotstuff_request);
      LOG(ERROR) << "CHATAY_HS1_TRACE primary_broadcast_next_nonblocking"
                 << " self=" << id_
                 << " type=" << HotStuffRequest_Type_Name(type)
                 << " next_type=TYPE_PRECOMMIT"
                 << " view=" << view_num
                 << " data_size=" << new_request->data().size();
      replica_communicator_->BroadCast(*new_request);
    } else {
      std::unique_ptr<Request> new_request = NewRequest(new_hotstuff_request);
      LOG(ERROR) << "CHATAY_HS1_TRACE primary_broadcast_next"
                 << " self=" << id_
                 << " type=" << HotStuffRequest_Type_Name(type)
                 << " view=" << view_num
                 << " data_size=" << new_request->data().size();
      replica_communicator_->BroadCast(*new_request);
    }
  }
  return 0;
}

int Commitment::ProcessMessageOnReplica(
    std::unique_ptr<HotStuffRequest> request) {
  LOG(ERROR) << "CHATAY_HS1_TRACE replica_enter"
             << " self=" << id_
             << " type=" << HotStuffRequest_Type_Name(request->type())
             << " request_view=" << request->view()
             << " node_view=" << request->node().info().view()
             << " qc_view=" << request->qc().node().info().view()
             << " sender=" << request->sender_id()
             << " current_view=" << current_view_;
  // LOG(ERROR) << "Replica receive type:"
  //            << HotStuffRequest_Type_Name(request->type())
  //            << " node view:" << request->node().info().view()
  //            << " qc view:" << request->qc().node().info().view()
  //            << " from:" << request->sender_id();
  if (request->type() == HotStuffRequest::TYPE_PRECOMMIT) {
    global_stats_->IncPropose();
  }
  if (request->type() == HotStuffRequest::TYPE_PREPARE) {
    received_senders_[(request->node().info().view() + 64) % 128].clear();
    received_requests_[(request->node().info().view() + 64) % 128].clear();
    global_stats_->IncPrepare();
  }
  if (request->type() == HotStuffRequest::TYPE_COMMIT) {
    global_stats_->IncCommit();
  }
  if (request->type() == HotStuffRequest::TYPE_DECIDE) {
    global_stats_->IncExecute();
  }
  if (request->type() == HotStuffRequest::TYPE_PREPARE) {
    // node's pre is highQC
    if (request->node().pre().hash() != request->qc().node().info().hash()) {
      LOG(ERROR) << "CHATAY_HS1_TRACE replica_reject_highqc_hash"
                 << " self=" << id_
                 << " type=" << HotStuffRequest_Type_Name(request->type())
                 << " request_view=" << request->view();
      LOG(ERROR) << "high qc hash not same";
      return -2;
    }
    if (!message_manager_->IsSaveNode(*request)) {
      LOG(ERROR) << "CHATAY_HS1_TRACE replica_reject_not_safe"
                 << " self=" << id_
                 << " type=" << HotStuffRequest_Type_Name(request->type())
                 << " request_view=" << request->view();
      LOG(ERROR) << "not safe node";
      return -2;
    }
  } else {
    if (!VerifyTS(*request)) {
      LOG(ERROR) << "CHATAY_HS1_TRACE replica_reject_ts"
                 << " self=" << id_
                 << " type=" << HotStuffRequest_Type_Name(request->type())
                 << " request_view=" << request->view()
                 << " qc_sigs=" << request->qc().signatures_size();
      LOG(ERROR) << " ts not invlid"
                 << " request type:"
                 << HotStuffRequest_Type_Name(request->type());
      return -2;
    }
  }

  // LOG(INFO) << "Replica receive type:"
  //           << HotStuffRequest_Type_Name(request->type())
  //           << " current view view:" << current_view_
  //           << " qc view:" << request->qc().node().info().view()
  //           << " from:" << request->sender_id();
  message_manager_->UpdateNode(*request);
  if (request->type() == HotStuffRequest::TYPE_DECIDE) {
    // execute and send new view
    LOG(ERROR) << "CHATAY_HS1_TRACE replica_decide_commit"
               << " self=" << id_
               << " view=" << request->qc().node().info().view()
               << " proxy=" << request->qc().node().proxy_id()
               << " data_size=" << request->qc().node().data().size();
    if (id_ == 1) {
      const int64_t replica_count = config_.GetReplicaInfos().size();
      const int64_t next_view = current_view_ + replica_count;
      LOG(ERROR) << "CHATAY_HS1_TRACE warm_cluster_entry_loop_evaluate"
                 << " self=" << id_
                 << " current_view=" << current_view_
                 << " decided_view=" << request->qc().node().info().view()
                 << " next_view=" << next_view
                 << " expected_primary=" << PrimaryId(next_view)
                 << " replica_count=" << replica_count
                 << " queue_empty_before=" << request_list_.Empty();
      std::thread([this, next_view, replica_count]() {
        const int64_t view_slot = next_view % 128;
        {
          std::unique_lock<std::mutex> lk(mutex_[view_slot]);
          received_senders_[view_slot].clear();
          received_requests_[view_slot].clear();
        }
        LOG(ERROR) << "CHATAY_HS1_TRACE warm_cluster_entry_loop_start"
                   << " self=" << id_
                   << " view=" << next_view
                   << " expected_primary=" << PrimaryId(next_view)
                   << " replica_count=" << replica_count
                   << " queue_empty_before_process=" << request_list_.Empty();
        const int ret = ProcessNewView(next_view);
        LOG(ERROR) << "CHATAY_HS1_TRACE warm_cluster_entry_loop_finish"
                   << " self=" << id_
                   << " view=" << next_view
                   << " ret=" << ret
                   << " queue_empty_after_process=" << request_list_.Empty();
      }).detach();
    }
    message_manager_->Commit(std::move(request));
  } else {
    // For prepare, send back the new node in qc. Then every message in the
    // future, the new node will be included in QC.
    if (!request->has_node()) {
      *request->mutable_node() = request->qc().node();
    }
    request->set_view(request->node().info().view());

    if (verifier_) {
      std::string data;
      request->node().SerializeToString(&data);
      auto signature_or = verifier_->SignMessage(data);
      if (!signature_or.ok()) {
        LOG(ERROR) << "Sign message fail";
        return -2;
      }
      *request->mutable_node_signature() = *signature_or;
    }
    // For ChainHotStuff, forward the lockQC as genericQC
    if (request->type() == HotStuffRequest::TYPE_PREPARE) {
      *request->mutable_qc() = message_manager_->GetPrepareQC();
    }
    std::unique_ptr<Request> vote_request =
        NewRequest(*request, VoteType(request->type()));
    // send vote to the primary.
    // LOG(INFO) << "send type:"
    //           << HotStuffRequest_Type_Name(VoteType(request->type()))
    //           << " to:" << PrimaryId(request->sender_id() + 1) << " view"
    //           << request->node().info().view();
    LOG(ERROR) << "CHATAY_HS1_TRACE replica_send_vote"
               << " self=" << id_
               << " vote_type="
               << HotStuffRequest_Type_Name(VoteType(request->type()))
               << " source_type="
               << HotStuffRequest_Type_Name(request->type())
               << " target_primary=" << PrimaryId(request->sender_id() + 1)
               << " view=" << request->node().info().view()
               << " data_size=" << vote_request->data().size();
    replica_communicator_->SendMessage(*vote_request,
                                       PrimaryId(request->sender_id() + 1));
  }
  return 0;
}

}  // namespace hotstuff
}  // namespace resdb
