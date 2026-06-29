# Patch Manifest - v2.14.2-alpha.1

## Source

Patches were exported from Chatay:

```txt
C:\Users\csjlo\OneDrive\Documentos\ResilientDB v0 (prueba)\infra\resilientdb\patches
```

into this fork:

```txt
documents/chatay-porting/v2.14.2-alpha.1/patches
```

## Excluded Utility Patches

The following Chatay patches are intentionally excluded from this consensus
porting package:

| File | Reason |
| --- | --- |
| `graphql-crow-disable-startup-block-scan.patch` | GraphQL/Crow startup utility, not consensus. |
| `graphql-workspace-nlohmann.patch` | Workspace dependency helper, not consensus. |

## HS1/PR100 Patches

Recommended handling:

1. Apply initial PR100/HotStuff baseline integration patches.
2. Apply runtime repair patches in chronological order.
3. Treat Python scripts as code-mod helpers, not direct `git apply` patches.
4. Re-run Bazel/build checks after each small group.
5. Instrument cold-start before optimizing NEWVIEW timing further.

| Patch | Bytes | SHA-256 |
| --- | ---: | --- |
| `patches/hs1-pr100/pr100-hotstuff-benchmark.patch` | 3667 | `9516aece4be363d4f7038c2c7475f089fc7f4726232c4706b8271d078b19594e` |
| `patches/hs1-pr100/pr100-hotstuff-kv-service.patch` | 1817 | `6c2412734dfd949130b9404188a3c784f71c1680c315cb987a6b060a2d39bd6a` |
| `patches/hs1-pr100/pr100-hotstuff-trace-instrumentation.patch` | 17551 | `df5e9e33f07b31a65a510f64620ea75289f47daf1116a555ff0409f89914ee02` |
| `patches/hs1-pr100/pr100-hotstuff-v281-runtime-repair.patch` | 1920 | `ff67e4cad5842188cf8dfb21612fed42d8df63240e7be47ddbd9076f0472dc32` |
| `patches/hs1-pr100/pr100-hotstuff-v282-nonblocking-phase.patch` | 2260 | `2469d6f595e0a801099a4c0b70258aa35b9f2f2d8fff736a64db3e1f1b0499a0` |
| `patches/hs1-pr100/pr100-hotstuff-v283-newview-bootstrap-hardening.patch` | 1536 | `f2bc602ff50335c8484e52211419e89c4086676ec406513db89e531b3d61b54f` |
| `patches/hs1-pr100/pr100-hotstuff-v284-staggered-newview-bootstrap.patch` | 1446 | `53545371b231dc830efb254fcd3e3abfff139f69360af90328bcd8fd815eb37d` |
| `patches/hs1-pr100/pr100-hotstuff-v285-warm-cluster-continuity-post-commit.patch` | 1151 | `69b054d3176cd29700b1ed741338bcc8679a07ada5e03dd9e9566780ef49a568` |
| `patches/hs1-pr100/pr100-hotstuff-v286-warm-cluster-continuity-async.patch` | 1462 | `df646636f83db1fa620591c82bb3417c31edea0e9b6d635bb005442804f78fa3` |
| `patches/hs1-pr100/pr100-hotstuff-v287-warm-cluster-entry-primary-loop.patch` | 1575 | `e55c333f6a444cbf67f6036ecfd934db5caf8b5c073761db398382fdd3bdebcb` |
| `patches/hs1-pr100/pr100-hotstuff-v288-warm-cluster-full-trace-entry-loop.patch` | 5926 | `2751d4bc5a74fe317534d8372c10bca7120a06efbb1fa415ceff2d50d5b15bc2` |
| `patches/hs1-pr100/pr100-hotstuff-v289-safe-highqc-trace.py` | 3581 | `607b75310aaf3dbb9f0a5c5de706c8571634b4dffa8e263131615fe748f51a15` |
| `patches/hs1-pr100/pr100-hotstuff-v290-compact-execution-seq.py` | 2304 | `5340e0afdf917022b33255357b2d4bc8c5680e6ad6ce9e3cde825aa84ec0fe9c` |

## HS2 Patches

Recommended handling:

1. Recreate the skeleton first.
2. Apply safety prototype before runtime wiring.
3. Add KV/build path.
4. Add client request execution.
5. Add pacemaker, leader recovery, timeout certificate, NewView/highQC and
   negative safety tests.

| Patch | Bytes | SHA-256 |
| --- | ---: | --- |
| `patches/hs2/hs2-fork-skeleton.patch` | 6742 | `3fbd084a70ba58d6156eaa972f9b662011118d6c308b4cf523f9da60eae703a9` |
| `patches/hs2/hs2-safety-prototype.patch` | 5768 | `748a832646b5b4826228e2ef5f2e78a2930901c297f5e96376a34007eef35a32` |
| `patches/hs2/hs2-resilientdb-wiring.patch` | 10277 | `cf9de0c889b5e1fb4b279119f863e62cbbac5f6f6c28572ea40859a0c12d2412` |
| `patches/hs2/hs2-kv-service-build.patch` | 1922 | `7f09e490347f9bcb7204e86c193f6764dd1042f7b999b7b0ee99b96aae2d4117` |
| `patches/hs2/hs2-client-request-runtime.patch` | 9546 | `76d1c4fa2574a81810074e59c2003a3c19b086a74159df102b2446fc38ab17fa` |
| `patches/hs2/hs2-pacemaker-probe.patch` | 6551 | `5652bf4df823afa8af661db011101853c997e323f13b99314de94d9c7516f28f` |
| `patches/hs2/hs2-leader-recovery-runtime.patch` | 9957 | `2541520d4868c49b6932b831e9685bdc8e6ab119cc6b21fc1fd3a58d5cc37863` |
| `patches/hs2/hs2-timeout-certificate-runtime.patch` | 14848 | `c2d4935128a5d987fbac77dd604f9dd1046e4feb6cccb4fb29b876909d0d9291` |
| `patches/hs2/hs2-new-view-highqc-runtime.patch` | 15628 | `3e4138a6ac6316599952c756653e3f518dc2ca772fb85629ee6fb0925a09bc8d` |
| `patches/hs2/hs2-negative-safety-tests.patch` | 9257 | `83edecd7f232cb39da54fa0e3b6cc66ed62535a35fa8047b7ddbed1fc94d1683` |

## Claim Boundary

This manifest proves that the patch set is now preserved inside the fork. It
does not prove that all patches still apply cleanly to current upstream
ResilientDB.

Clean application and layout reconciliation belong to:

```txt
v2.14.3-alpha.1
v2.14.4-alpha.1
```
