/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "platform/networkstrate/service_network.h"

#include <glog/logging.h>
#include <signal.h>

#include <chrono>
#include <thread>

#include "platform/common/network/tcp_socket.h"
#include "platform/proto/broadcast.pb.h"

namespace resdb {

namespace {

int64_t ColdStartNowMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

}  // namespace

ServiceNetwork::ServiceNetwork(const ResDBConfig& config,
                               std::unique_ptr<ServiceInterface> service)
    : service_(std::move(service)),
      input_queue_("input"),
      resp_queue_("resp"),
      config_(config) {
  const int64_t started_at = ColdStartNowMs();
  LOG(ERROR) << "CHATAY_HS1_COLD_START service_network_ctor_start"
             << " ts_ms=" << started_at
             << " self=" << config_.GetSelfInfo().id()
             << " port=" << config_.GetSelfInfo().port();
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  if (sigemptyset(&sa.sa_mask) == -1 || sigaction(SIGPIPE, &sa, 0) == -1) {
    perror("failed to ignore SIGPIPE; sigaction");
    exit(EXIT_FAILURE);
  }

  acceptor_ = std::make_unique<Acceptor>(config, &input_queue_);
  LOG(ERROR) << "CHATAY_HS1_COLD_START service_network_acceptor_created"
             << " ts_ms=" << ColdStartNowMs()
             << " self=" << config_.GetSelfInfo().id()
             << " acceptor_port=" << config_.GetSelfInfo().port();

  async_acceptor_ = std::make_unique<AsyncAcceptor>(
      config.GetSelfInfo().ip(), config_.GetSelfInfo().port() + 10000,
      config.GetInputWorkerNum(),
      std::bind(&ServiceNetwork::AcceptorHandler, this, std::placeholders::_1,
                std::placeholders::_2));
  async_acceptor_->StartAccept();
  LOG(ERROR) << "CHATAY_HS1_COLD_START service_network_async_acceptor_started"
             << " ts_ms=" << ColdStartNowMs()
             << " self=" << config_.GetSelfInfo().id()
             << " async_port=" << config_.GetSelfInfo().port() + 10000;
  global_stats_ = Stats::GetGlobalStats();
  LOG(ERROR) << "CHATAY_HS1_COLD_START service_network_ctor_finish"
             << " ts_ms=" << ColdStartNowMs()
             << " duration_ms=" << (ColdStartNowMs() - started_at)
             << " self=" << config_.GetSelfInfo().id();
}

ServiceNetwork::~ServiceNetwork() {}

void ServiceNetwork::AcceptorHandler(const char* buffer, size_t data_len) {
  BroadcastData data;
  if (!data.ParseFromArray(buffer, data_len)) {
    LOG(ERROR) << "parse broad cast fail:" << data_len;
    return;
  }

  for (auto& sub_data : data.data()) {
    std::unique_ptr<DataInfo> sub_request_info = std::make_unique<DataInfo>();
    sub_request_info->data_len = sub_data.size();
    sub_request_info->buff = new char[sub_request_info->data_len];
    memcpy(sub_request_info->buff, sub_data.data(), sub_request_info->data_len);
    std::unique_ptr<QueueItem> item = std::make_unique<QueueItem>();
    item->socket = nullptr;
    item->data = std::move(sub_request_info);
    // LOG(ERROR) << "receve data from acceptor:" << data.is_resp()<<" data
    // len:"<<item->data->data_len;
    global_stats_->ServerCall();
    input_queue_.Push(std::move(item));
  }
}

void ServiceNetwork::InputProcess() {
  std::vector<std::thread> threads;

  int woker_num = config_.GetWorkerNum();
  LOG(ERROR) << "CHATAY_HS1_COLD_START input_process_start"
             << " ts_ms=" << ColdStartNowMs()
             << " self=" << config_.GetSelfInfo().id()
             << " worker_num=" << woker_num;
  LOG(ERROR) << "server:" << config_.GetSelfInfo().id() << " start running";
  for (int i = 0; i < woker_num; ++i) {
    threads.push_back(std::thread([&]() {
      while (IsRunning()) {
        std::unique_ptr<QueueItem> item = input_queue_.Pop(1000);
        if (item == nullptr) {
          continue;
        }
        global_stats_->ServerProcess();
        Process(std::move(item));
      }
    }));
  }

  for (auto& th : threads) {
    if (th.joinable()) {
      th.join();
    }
  }
}

void ServiceNetwork::Run() {
  LOG(ERROR) << "CHATAY_HS1_COLD_START service_network_run_enter"
             << " ts_ms=" << ColdStartNowMs()
             << " self=" << config_.GetSelfInfo().id();
  service_->Start();
  LOG(ERROR) << "CHATAY_HS1_COLD_START service_network_service_started"
             << " ts_ms=" << ColdStartNowMs()
             << " self=" << config_.GetSelfInfo().id();
  auto input_th = std::thread(&ServiceNetwork::InputProcess, this);
  LOG(ERROR) << "CHATAY_HS1_COLD_START service_network_input_thread_started"
             << " ts_ms=" << ColdStartNowMs()
             << " self=" << config_.GetSelfInfo().id();

  LOG(ERROR) << "CHATAY_HS1_COLD_START service_network_acceptor_run_enter"
             << " ts_ms=" << ColdStartNowMs()
             << " self=" << config_.GetSelfInfo().id();
  acceptor_->Run();

  input_th.join();
}

// Receive a message from network and pass it to service to process.
void ServiceNetwork::Process(std::unique_ptr<QueueItem> item) {
  auto client_socket =
      item->socket == nullptr ? nullptr : std::move(item->socket);
  auto request_info = std::move(item->data);
  if (client_socket != nullptr) {
    client_socket->SetSendTimeout(1000000);
  }
  std::unique_ptr<Context> context = std::make_unique<Context>();
  context->client = std::make_unique<NetChannel>(std::move(client_socket),
                                                 /*connected=*/true);
  if (request_info) {
    service_->Process(std::move(context), std::move(request_info));
  }
  return;
}

bool ServiceNetwork::IsRunning() { return service_->IsRunning(); }

void ServiceNetwork::Stop() {
  acceptor_->Stop();
  service_->Stop();
}

bool ServiceNetwork::ServiceIsReady() const { return service_->IsReady(); }

}  // namespace resdb
