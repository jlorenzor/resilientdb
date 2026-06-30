# Current ResilientDB Layout Map - v2.14.3-alpha.1

## Base

Fork branch:

```txt
consensus/layout-map-v2.14.3-alpha.1
```

Base porting package:

```txt
documents/chatay-porting/v2.14.2-alpha.1
```

Current upstream base commit:

```txt
ce65a1ed7d3789d11fa8f154544d1bd31607fae5
```

## Consensus Directories Present

| Directory | Status | Notes |
| --- | --- | --- |
| `platform/consensus/ordering/pbft` | present | Main PBFT implementation used by current ResilientDB services. |
| `platform/consensus/ordering/geo_pbft` | present | GeoBFT/GeoPBFT path, not part of the immediate HS1/HS2 port. |
| `platform/consensus/ordering/poc` | present | Proof-of-concept consensus paths. |
| `platform/consensus/ordering/poe` | present | Useful modular reference for adding new protocol directories. |
| `platform/consensus/ordering/common` | present | Shared framework/algorithm utilities. |
| `platform/consensus/ordering/hotstuff` | absent | Required for HS1/PR100. Must be recreated from PR100 or patch source. |
| `platform/consensus/ordering/hs2` | absent | Required for HS2. Must be created before applying HS2 patches. |

## Benchmark Protocol Directories Present

| Directory | Status | Notes |
| --- | --- | --- |
| `benchmark/protocols/pbft` | present | Contains PBFT KV benchmark binaries. |
| `benchmark/protocols/poe` | present | Contains PoE KV benchmark binary. Useful template for HS2/HS1 benchmark directories. |
| `benchmark/protocols/hotstuff` | absent | Required for HS1/PR100 KV benchmark and service tools. |
| `benchmark/protocols/hs2` | absent | Required for HS2 probes and KV service. |

## KV And Service Entry Points

| Path | Status | Role |
| --- | --- | --- |
| `service/kv/kv_service.cpp` | present | Main KV service entry point using default PBFT path through `GenerateResDBServer`. |
| `service/tools/kv/api_tools/kv_service_tools.cpp` | present | Client/API tool for KV requests. |
| `benchmark/protocols/pbft/kv_server_performance.cpp` | present | PBFT benchmark server using `ConsensusManagerPBFT`. |
| `benchmark/protocols/pbft/kv_service_tools.cpp` | present | PBFT benchmark KV client tools. |
| `benchmark/protocols/poe/kv_server_performance.cpp` | present | PoE benchmark server using `poe/framework:consensus`. |
| `executor/kv/kv_executor.cpp` | present | KV executor used by services and benchmark protocol runners. |
| `proto/kv/kv.proto` | present | KV request/response protobuf. |

## PBFT Baseline Map

| Component | Path / Target |
| --- | --- |
| Consensus manager | `platform/consensus/ordering/pbft/consensus_manager_pbft.{h,cpp}` |
| Commitment | `platform/consensus/ordering/pbft/commitment.{h,cpp}` |
| Message manager | `platform/consensus/ordering/pbft/message_manager.{h,cpp}` |
| Response manager | `platform/consensus/ordering/pbft/response_manager.{h,cpp}` |
| View change | `platform/consensus/ordering/pbft/viewchange_manager.{h,cpp}` |
| Bazel target | `//platform/consensus/ordering/pbft:consensus_manager_pbft` |
| KV benchmark | `//benchmark/protocols/pbft:kv_server_performance` |
| KV client tool | `//benchmark/protocols/pbft:kv_service_tools` |

## PoE As Template For New Protocols

PoE is important because it already follows a modular protocol layout:

```txt
platform/consensus/ordering/poe/algorithm
platform/consensus/ordering/poe/framework
platform/consensus/ordering/poe/proto
benchmark/protocols/poe
```

HS2 should prefer this modular shape over copying PBFT's monolithic directory
unless a PBFT-specific execution hook is required.

## Service Factory Hook

`service/utils/server_factory.h` already supports template-based protocol
selection:

```cpp
template <typename ConsensusProtocol = ConsensusManagerPBFT>
std::unique_ptr<ServiceNetwork> CustomCreateResDBServer(...);
```

This is the cleanest hook for HS1 and HS2 runtime integration. A future HS1/HS2
KV service should use:

```cpp
CustomGenerateResDBServer<ConsensusProtocol>(...)
```

rather than changing the default PBFT service.

## Common Framework Reconciliation

Older patches refer to:

```txt
platform/consensus/ordering/common/response_manager.cpp
platform/consensus/ordering/common/message_manager_basic.cpp
```

Current upstream has:

```txt
platform/consensus/ordering/common/framework/response_manager.cpp
platform/consensus/ordering/common/framework/response_manager.h
platform/consensus/ordering/common/framework/consensus.cpp
platform/consensus/ordering/common/framework/consensus.h
```

Therefore, common-framework patches must be reviewed before direct application.
They are not safe to `git apply` blindly.

## Image Size Reminder

No images were built in this version. The `v2.14.2-alpha.1` policy remains:

```txt
expected runtime image size: 0.9 GB - 1.1 GB
review threshold: > 1.25 GB
```
