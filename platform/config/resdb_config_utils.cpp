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

#include "platform/config/resdb_config_utils.h"

#include <fcntl.h>
#include <glog/logging.h>
#include <google/protobuf/util/json_util.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <chrono>
#include <nlohmann/json.hpp>
#include <regex>

namespace resdb {

using ::google::protobuf::util::JsonParseOptions;
using json = nlohmann::json;

namespace {

int64_t ColdStartNowMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

KeyInfo ReadKey(const std::string& file_name) {
  const int64_t started_at = ColdStartNowMs();
  LOG(ERROR) << "CHATAY_HS1_COLD_START read_private_key_start"
             << " ts_ms=" << started_at
             << " file=" << file_name;
  int fd = open(file_name.c_str(), O_RDONLY, 0666);
  if (fd < 0) {
    LOG(ERROR) << "open file:" << file_name << " fail:" << strerror(errno);
  }
  assert(fd >= 0);

  std::string res;
  int read_len = 0;
  char tmp[1024];
  while (true) {
    read_len = read(fd, tmp, sizeof(tmp));
    if (read_len <= 0) {
      break;
    }
    res.append(tmp, read_len);
  }
  close(fd);
  KeyInfo key;
  assert(key.ParseFromString(res));
  LOG(ERROR) << "CHATAY_HS1_COLD_START read_private_key_finish"
             << " ts_ms=" << ColdStartNowMs()
             << " duration_ms=" << (ColdStartNowMs() - started_at)
             << " file=" << file_name
             << " bytes=" << res.size()
             << " hash_type=" << key.hash_type();
  return key;
}

CertificateInfo ReadCert(const std::string& file_name) {
  const int64_t started_at = ColdStartNowMs();
  LOG(ERROR) << "CHATAY_HS1_COLD_START read_cert_start"
             << " ts_ms=" << started_at
             << " file=" << file_name;
  int fd = open(file_name.c_str(), O_RDONLY, 0666);
  if (fd < 0) {
    LOG(ERROR) << "open file:" << file_name << " fail" << strerror(errno);
  }
  assert(fd >= 0);

  std::string res;
  int read_len = 0;
  char tmp[1024];
  while (true) {
    read_len = read(fd, tmp, sizeof(tmp));
    if (read_len <= 0) {
      break;
    }
    res.append(tmp, read_len);
  }
  close(fd);
  CertificateInfo info;
  assert(info.ParseFromString(res));
  LOG(ERROR) << "CHATAY_HS1_COLD_START read_cert_finish"
             << " ts_ms=" << ColdStartNowMs()
             << " duration_ms=" << (ColdStartNowMs() - started_at)
             << " file=" << file_name
             << " bytes=" << res.size()
             << " node_id=" << info.public_key().public_key_info().node_id()
             << " type=" << info.public_key().public_key_info().type();
  return info;
}

}  // namespace

ReplicaInfo GenerateReplicaInfo(int id, const std::string& ip, int port) {
  ReplicaInfo info;
  info.set_id(id);
  info.set_ip(ip);
  info.set_port(port);
  return info;
}

std::string RemoveJsonComments(const std::string& jsonWithComments) {
  std::string result =
      std::regex_replace(jsonWithComments, std::regex("/\\*.*?\\*/"), "");
  result = std::regex_replace(result, std::regex("//.*?\\n"), "\n");
  return result;
}

ResConfigData ReadConfigFromFile(const std::string& file_name) {
  const int64_t started_at = ColdStartNowMs();
  LOG(ERROR) << "CHATAY_HS1_COLD_START read_config_start"
             << " ts_ms=" << started_at
             << " file=" << file_name;
  std::stringstream json_data;
  std::ifstream infile(file_name.c_str());
  if (!infile.is_open()) {
    std::cerr << "Failed to open file." << file_name << " " << strerror(errno)
              << std::endl;
    return ResConfigData();
  }

  json_data << infile.rdbuf();
  std::string cleanJson = RemoveJsonComments(json_data.str());

  ResConfigData config_data;
  JsonParseOptions options;
  auto status = JsonStringToMessage(cleanJson, &config_data, options);
  if (!status.ok()) {
    LOG(ERROR) << "parse json :" << file_name << " fail:" << status.message();
  }
  assert(status.ok());
  LOG(ERROR) << "CHATAY_HS1_COLD_START read_config_finish"
             << " ts_ms=" << ColdStartNowMs()
             << " duration_ms=" << (ColdStartNowMs() - started_at)
             << " file=" << file_name
             << " bytes=" << cleanJson.size()
             << " regions=" << config_data.region_size();
  return config_data;
}

std::vector<ReplicaInfo> ReadConfig(const std::string& file_name) {
  std::vector<ReplicaInfo> replicas;
  std::stringstream json_data;
  std::ifstream infile(file_name.c_str());
  if (!infile.is_open()) {
    std::cerr << "Failed to open file." << file_name << " " << strerror(errno)
              << std::endl;
    return replicas;
  }

  json_data << infile.rdbuf();
  std::string cleanJson = RemoveJsonComments(json_data.str());

  RegionInfo region_info;
  JsonParseOptions options;
  auto status = JsonStringToMessage(cleanJson, &region_info, options);
  if (!status.ok()) {
    LOG(ERROR) << "parse json :" << file_name << " fail:" << status.message();
  }
  assert(status.ok());
  for (const auto& replica_info : region_info.replica_info()) {
    LOG(ERROR) << "parse json id:" << replica_info.id()
               << " ip:" << replica_info.ip()
               << " port:" << replica_info.port();
    replicas.push_back(GenerateReplicaInfo(replica_info.id(), replica_info.ip(),
                                           replica_info.port()));
  }
  return replicas;
}

std::unique_ptr<ResDBConfig> GenerateResDBConfig(
    const std::string& config_file, const std::string& private_key_file,
    const std::string& cert_file, std::optional<ReplicaInfo> self_info,
    std::optional<ConfigGenFunc> gen_func) {
  const int64_t started_at = ColdStartNowMs();
  LOG(ERROR) << "CHATAY_HS1_COLD_START generate_resdb_config_start"
             << " ts_ms=" << started_at
             << " config_file=" << config_file
             << " private_key_file=" << private_key_file
             << " cert_file=" << cert_file;
  ResConfigData config_data = ReadConfigFromFile(config_file);
  KeyInfo private_key = ReadKey(private_key_file);
  CertificateInfo cert_info = ReadCert(cert_file);

  LOG(ERROR) << "CHATAY_HS1_COLD_START key_cert_loaded"
             << " ts_ms=" << ColdStartNowMs()
             << " node_id=" << cert_info.public_key().public_key_info().node_id()
             << " public_key_hash_type="
             << cert_info.public_key().public_key_info().key().hash_type()
             << " private_key_hash_type=" << private_key.hash_type();
  if (!self_info.has_value()) {
    self_info = ReplicaInfo();
  }

  (*self_info).set_id(cert_info.public_key().public_key_info().node_id());
  (*self_info).set_ip(cert_info.public_key().public_key_info().ip());
  (*self_info).set_port(cert_info.public_key().public_key_info().port());

  *(*self_info).mutable_certificate_info() = cert_info;

  if (gen_func.has_value()) {
    LOG(ERROR) << "CHATAY_HS1_COLD_START generate_resdb_config_finish"
               << " ts_ms=" << ColdStartNowMs()
               << " duration_ms=" << (ColdStartNowMs() - started_at)
               << " self=" << self_info->id()
               << " custom_gen=1";
    return (*gen_func)(config_data, self_info.value(), private_key, cert_info);
  }
  LOG(ERROR) << "CHATAY_HS1_COLD_START generate_resdb_config_finish"
             << " ts_ms=" << ColdStartNowMs()
             << " duration_ms=" << (ColdStartNowMs() - started_at)
             << " self=" << self_info->id()
             << " custom_gen=0";
  return std::make_unique<ResDBConfig>(config_data, self_info.value(),
                                       private_key, cert_info);
}

ResDBConfig GenerateResDBConfig(const std::string& config_file) {
  std::vector<ReplicaInfo> replicas = ReadConfig(config_file);
  return ResDBConfig(replicas, ReplicaInfo());
}

}  // namespace resdb
