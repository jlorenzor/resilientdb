#include <iostream>
#include <vector>

#include "platform/config/resdb_config.h"
#include "platform/consensus/execution/system_info.h"
#include "platform/consensus/ordering/hs2/hs2_pacemaker.h"
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
  info.set_port(19000 + id);
  return info;
}

}  // namespace

int main() {
  std::vector<resdb::ReplicaInfo> replicas;
  for (int id = 1; id <= 4; ++id) {
    replicas.push_back(MakeReplica(id));
  }

  resdb::ResDBConfig config(replicas, replicas.front());
  resdb::SystemInfo system_info(config);
  resdb::hs2::Hs2Pacemaker pacemaker(config, &system_info);

  if (pacemaker.CurrentView() != 1) {
    return Fail("unexpected initial view");
  }
  if (pacemaker.CurrentLeader() != 1) {
    return Fail("unexpected initial leader");
  }
  if (pacemaker.LeaderForView(1) != 1) {
    return Fail("view 1 leader mismatch");
  }
  if (pacemaker.LeaderForView(2) != 2) {
    return Fail("view 2 leader mismatch");
  }
  if (pacemaker.LeaderForView(3) != 3) {
    return Fail("view 3 leader mismatch");
  }
  if (pacemaker.LeaderForView(4) != 4) {
    return Fail("view 4 leader mismatch");
  }
  if (pacemaker.LeaderForView(5) != 1) {
    return Fail("view 5 leader mismatch");
  }
  if (pacemaker.AdvanceView(1)) {
    return Fail("stale view accepted");
  }
  if (!pacemaker.AdvanceView(2)) {
    return Fail("view 2 rejected");
  }
  if (system_info.GetPrimaryId() != 2 || system_info.GetCurrentView() != 2) {
    return Fail("system info not updated for view 2");
  }
  if (!pacemaker.AdvanceView(4)) {
    return Fail("view 4 rejected");
  }
  if (system_info.GetPrimaryId() != 4 || system_info.GetCurrentView() != 4) {
    return Fail("system info not updated for view 4");
  }
  if (!pacemaker.AdvanceView(5)) {
    return Fail("view 5 rejected");
  }
  if (system_info.GetPrimaryId() != 1 || system_info.GetCurrentView() != 5) {
    return Fail("system info not updated for view 5");
  }

  std::cout << "hs2 pacemaker ok" << std::endl;
  return 0;
}
