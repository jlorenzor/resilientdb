#include <cstdio>
#include <cstdlib>
#include <memory>
#include <chrono>

#include <glog/logging.h>

#include "chain/storage/memory_db.h"
#include "executor/kv/kv_executor.h"
#include "platform/config/resdb_config_utils.h"
#include "platform/consensus/ordering/hotstuff/consensus.h"
#include "platform/networkstrate/service_network.h"

using namespace resdb;
using namespace resdb::storage;

namespace {

int64_t ColdStartNowMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

}  // namespace

void ShowUsage() {
  printf("<config> <private_key> <cert_file> [logging_dir]\n");
}

int main(int argc, char** argv) {
  const int64_t process_start_ms = ColdStartNowMs();
  LOG(ERROR) << "CHATAY_HS1_COLD_START process_main_enter"
             << " ts_ms=" << process_start_ms
             << " argc=" << argc;
  if (argc < 4) {
    ShowUsage();
    exit(0);
  }

  char* config_file = argv[1];
  char* private_key_file = argv[2];
  char* cert_file = argv[3];
  LOG(ERROR) << "CHATAY_HS1_COLD_START config_load_start"
             << " ts_ms=" << ColdStartNowMs()
             << " config_file=" << config_file
             << " private_key_file=" << private_key_file
             << " cert_file=" << cert_file;
  std::unique_ptr<ResDBConfig> config =
      GenerateResDBConfig(config_file, private_key_file, cert_file);
  LOG(ERROR) << "CHATAY_HS1_COLD_START config_load_finish"
             << " ts_ms=" << ColdStartNowMs()
             << " elapsed_ms=" << (ColdStartNowMs() - process_start_ms)
             << " self=" << config->GetSelfInfo().id()
             << " port=" << config->GetSelfInfo().port()
             << " replicas=" << config->GetReplicaInfos().size();

  LOG(ERROR) << "CHATAY_HS1_COLD_START consensus_construct_start"
             << " ts_ms=" << ColdStartNowMs()
             << " self=" << config->GetSelfInfo().id();
  auto consensus = std::make_unique<hotstuff::Consensus>(
      *config, std::make_unique<KVExecutor>(std::make_unique<MemoryDB>()));
  LOG(ERROR) << "CHATAY_HS1_COLD_START consensus_construct_finish"
             << " ts_ms=" << ColdStartNowMs()
             << " self=" << config->GetSelfInfo().id();

  LOG(ERROR) << "CHATAY_HS1_COLD_START service_network_construct_start"
             << " ts_ms=" << ColdStartNowMs()
             << " self=" << config->GetSelfInfo().id();
  auto server =
      std::make_unique<ServiceNetwork>(*config, std::move(consensus));
  LOG(ERROR) << "CHATAY_HS1_COLD_START service_network_construct_finish"
             << " ts_ms=" << ColdStartNowMs()
             << " self=" << config->GetSelfInfo().id();
  LOG(ERROR) << "CHATAY_HS1_COLD_START server_run_enter"
             << " ts_ms=" << ColdStartNowMs()
             << " self=" << config->GetSelfInfo().id();
  server->Run();
}
