#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "platform/config/resdb_config.h"
#include "platform/consensus/ordering/hs2/consensus_manager_hs2.h"
#include "platform/proto/resdb.pb.h"

namespace {

int Fail(const char* message) {
  std::cerr << message << std::endl;
  return 1;
}

resdb::ReplicaInfo MakeReplica(int id) {
  resdb::ReplicaInfo info;
  info.set_id(id);
  info.set_ip("127.0.0.1");
  info.set_port(17000 + id);
  return info;
}

}  // namespace

int main() {
  std::vector<resdb::ReplicaInfo> replicas;
  for (int id = 1; id <= 4; ++id) {
    replicas.push_back(MakeReplica(id));
  }

  resdb::ResDBConfig config(replicas, replicas.front());
  config.SetSignatureVerifierEnabled(false);
  config.SetHeartBeatEnabled(false);
  config.SetTestMode(true);

  resdb::hs2::ConsensusManagerHs2 manager(config);

  if (std::string(manager.ProtocolName()) != "hs2") {
    return Fail("unexpected protocol name");
  }
  if (manager.GetReplicas().size() != 4) {
    return Fail("unexpected replica count");
  }
  if (manager.GetPrimary() != 1) {
    return Fail("unexpected initial leader");
  }
  if (manager.GetVersion() != 1) {
    return Fail("unexpected initial view");
  }
  if (manager.Core().ReplicaCount() != 4) {
    return Fail("unexpected core replica count");
  }
  if (manager.Core().QuorumSize() != 3) {
    return Fail("unexpected core quorum");
  }
  if (!manager.SupportsRequestType(resdb::Request::TYPE_CLIENT_REQUEST)) {
    return Fail("client request type not supported");
  }
  if (!manager.SupportsRequestType(resdb::Request::TYPE_NEW_TXNS)) {
    return Fail("new txns type not supported");
  }
  if (!manager.SupportsRequestType(resdb::Request::TYPE_CUSTOM_QUERY)) {
    return Fail("custom query type not supported");
  }
  if (!manager.SupportsRequestType(resdb::Request::TYPE_CUSTOM_CONSENSUS)) {
    return Fail("custom consensus type not supported");
  }
  if (manager.SupportsRequestType(resdb::Request::TYPE_HEART_BEAT)) {
    return Fail("heartbeat should stay in base ConsensusManager dispatch");
  }

  manager.SetPrimary(2, 2);
  if (manager.GetPrimary() != 2 || manager.GetVersion() != 2) {
    return Fail("leader/view update failed");
  }

  std::cout << "hs2 wiring ok" << std::endl;
  return 0;
}
