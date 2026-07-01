#include <cstdio>
#include <cstdlib>
#include <memory>

#include "chain/storage/memory_db.h"
#include "executor/kv/kv_executor.h"
#include "platform/config/resdb_config_utils.h"
#include "platform/consensus/ordering/pbft/consensus_manager_pbft.h"
#include "platform/networkstrate/service_network.h"

using namespace resdb;
using namespace resdb::storage;

namespace {

void ShowUsage() {
  printf("<config> <private_key> <cert_file> [logging_dir]\n");
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 4) {
    ShowUsage();
    exit(0);
  }

  char* config_file = argv[1];
  char* private_key_file = argv[2];
  char* cert_file = argv[3];

  std::unique_ptr<ResDBConfig> config =
      GenerateResDBConfig(config_file, private_key_file, cert_file);

  auto consensus = std::make_unique<ConsensusManagerPBFT>(
      *config, std::make_unique<KVExecutor>(std::make_unique<MemoryDB>()));

  auto server =
      std::make_unique<ServiceNetwork>(*config, std::move(consensus));
  server->Run();
}
