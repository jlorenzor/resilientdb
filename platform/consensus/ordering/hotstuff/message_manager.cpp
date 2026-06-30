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

#include "platform/consensus/ordering/hotstuff/message_manager.h"

#include <glog/logging.h>

namespace resdb {
namespace hotstuff {

MessageManager::MessageManager(
    const ResDBConfig& config,
    std::unique_ptr<TransactionManager> executor_impl, SystemInfo* system_info)
    : MessageManagerBasic(config, std::move(executor_impl), system_info) {}

int MessageManager::UpdateNode(const HotStuffRequest& request) {
  if (request.type() == HotStuffRequest::TYPE_COMMIT) {
    SetLockQC(request.qc());
  } else if (request.type() == HotStuffRequest::TYPE_PRECOMMIT) {
    SetPrepareQC(request.qc());
  }
  return 0;
}

void MessageManager::SetPrepareQC(const QC& qc) {
  std::unique_lock<std::mutex> lk(mutex_);
  prepare_qc_ = qc;
}

QC MessageManager::GetPrepareQC() {
  std::unique_lock<std::mutex> lk(mutex_);
  return prepare_qc_;
}

void MessageManager::SetLockQC(const QC& qc) {
  std::unique_lock<std::mutex> lk(mutex_);
  lock_qc_ = qc;
}

bool MessageManager::IsSaveNode(const HotStuffRequest& request) {
  std::unique_lock<std::mutex> lk(mutex_);
  const bool pre_hash_matches =
      request.node().pre().hash() == lock_qc_.node().info().hash();
  const bool qc_view_is_newer =
      request.qc().node().info().view() > lock_qc_.node().info().view();
  LOG(ERROR) << "CHATAY_HS1_TRACE safe_node_check"
             << " request_node_view=" << request.node().info().view()
             << " qc_view=" << request.qc().node().info().view()
             << " lock_qc_view=" << lock_qc_.node().info().view()
             << " pre_hash_size=" << request.node().pre().hash().size()
             << " lock_hash_size=" << lock_qc_.node().info().hash().size()
             << " pre_hash_matches=" << pre_hash_matches
             << " qc_view_is_newer=" << qc_view_is_newer;
  return pre_hash_matches || qc_view_is_newer;
}

int MessageManager::Commit(std::unique_ptr<HotStuffRequest> hotstuff_request) {
  std::unique_ptr<Request> execute_request = std::make_unique<Request>();
  const int64_t consensus_view = hotstuff_request->qc().node().info().view();
  int64_t execute_seq = 0;
  {
    std::unique_lock<std::mutex> lk(mutex_);
    execute_seq = next_execute_seq_++;
  }
  execute_request->set_data(hotstuff_request->qc().node().data());
  execute_request->set_seq(execute_seq);
  execute_request->set_proxy_id(hotstuff_request->qc().node().proxy_id());
  LOG(ERROR) << "CHATAY_HS1_TRACE message_manager_commit"
             << " execute_seq=" << execute_request->seq()
             << " consensus_view=" << consensus_view
             << " proxy=" << execute_request->proxy_id()
             << " data_size=" << execute_request->data().size();
  transaction_executor_->Commit(std::move(execute_request));
  return 0;
}

}  // namespace hotstuff
}  // namespace resdb
