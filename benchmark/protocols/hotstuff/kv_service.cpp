#include <cstdio>
#include <cstdlib>
#include <memory>

#include "chain/state/chain_state.h"
#include "executor/kv/kv_executor.h"
#include "platform/config/resdb_config_utils.h"
#include "platform/consensus/ordering/hotstuff/consensus.h"
#include "platform/networkstrate/service_network.h"

using namespace resdb;

void ShowUsage() {
  printf("<config> <private_key> <cert_file> [logging_dir]\n");
}

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

  auto consensus = std::make_unique<hotstuff::Consensus>(
      *config, std::make_unique<KVExecutor>(std::make_unique<ChainState>()));

  auto server =
      std::make_unique<ServiceNetwork>(*config, std::move(consensus));
  server->Run();
}
