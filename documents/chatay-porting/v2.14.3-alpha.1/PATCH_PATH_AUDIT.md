# Patch Path Audit - v2.14.3-alpha.1

## Summary

The audit inspected `diff --git` paths from the exported
`v2.14.2-alpha.1` patch package against the current fork tree.

| Group | Existing target | New file in existing directory | New tree required |
| --- | ---: | ---: | ---: |
| HS1/PR100 | 0 | 1 | 16 |
| HS2 | 0 | 5 | 40 |

## Interpretation

The current ResilientDB fork does not contain the HS1/PR100 `hotstuff` tree or
the HS2 tree. Most patch paths therefore require creating protocol directories
before applying later repairs.

The few `NEW_FILE_IN_EXISTING_DIR` entries are not automatically safe. Some of
them point to paths that existed in older work but whose current equivalent
appears to live under `platform/consensus/ordering/common/framework`.

## HS1/PR100 Detailed Audit

| Patch | Target | Status | Action |
| --- | --- | --- | --- |
| `pr100-hotstuff-benchmark.patch` | `benchmark/protocols/hotstuff/BUILD` | new tree required | Recover/create benchmark hotstuff directory. |
| `pr100-hotstuff-benchmark.patch` | `benchmark/protocols/hotstuff/kv_server_performance.cpp` | new tree required | Recover/create benchmark hotstuff directory. |
| `pr100-hotstuff-kv-service.patch` | `benchmark/protocols/hotstuff/BUILD` | new tree required | Merge with benchmark BUILD once directory exists. |
| `pr100-hotstuff-kv-service.patch` | `benchmark/protocols/hotstuff/kv_service.cpp` | new tree required | Recover/create HS1 KV service. |
| `pr100-hotstuff-trace-instrumentation.patch` | `platform/consensus/ordering/common/response_manager.cpp` | new file in existing directory | Reconcile with `common/framework/response_manager.cpp`. |
| `pr100-hotstuff-trace-instrumentation.patch` | `platform/consensus/ordering/hotstuff/commitment.cpp` | new tree required | Requires HS1 baseline tree first. |
| `pr100-hotstuff-trace-instrumentation.patch` | `platform/consensus/ordering/hotstuff/consensus.cpp` | new tree required | Requires HS1 baseline tree first. |
| `pr100-hotstuff-trace-instrumentation.patch` | `platform/consensus/ordering/hotstuff/message_manager.cpp` | new tree required | Requires HS1 baseline tree first. |
| `pr100-hotstuff-v281-runtime-repair.patch` | `platform/consensus/ordering/hotstuff/commitment.cpp` | new tree required | Apply only after HS1 baseline tree. |
| `pr100-hotstuff-v281-runtime-repair.patch` | `platform/consensus/ordering/hotstuff/consensus.cpp` | new tree required | Apply only after HS1 baseline tree. |
| `pr100-hotstuff-v282-nonblocking-phase.patch` | `platform/consensus/ordering/hotstuff/commitment.cpp` | new tree required | Apply only after HS1 baseline tree. |
| `pr100-hotstuff-v283-newview-bootstrap-hardening.patch` | `platform/consensus/ordering/hotstuff/commitment.cpp` | new tree required | Historical negative/diagnostic patch; do not make final default. |
| `pr100-hotstuff-v284-staggered-newview-bootstrap.patch` | `platform/consensus/ordering/hotstuff/commitment.cpp` | new tree required | Current best HS1 bootstrap repair candidate. |
| `pr100-hotstuff-v285-warm-cluster-continuity-post-commit.patch` | `platform/consensus/ordering/hotstuff/commitment.cpp` | new tree required | Warm-cluster repair after baseline tree. |
| `pr100-hotstuff-v286-warm-cluster-continuity-async.patch` | `platform/consensus/ordering/hotstuff/commitment.cpp` | new tree required | Warm-cluster repair after baseline tree. |
| `pr100-hotstuff-v287-warm-cluster-entry-primary-loop.patch` | `platform/consensus/ordering/hotstuff/commitment.cpp` | new tree required | Warm-cluster repair after baseline tree. |
| `pr100-hotstuff-v288-warm-cluster-full-trace-entry-loop.patch` | `platform/consensus/ordering/hotstuff/commitment.cpp` | new tree required | Trace-rich repair after baseline tree. |

Python helpers:

| Helper | Action |
| --- | --- |
| `pr100-hotstuff-v289-safe-highqc-trace.py` | Run only after the HS1 tree exists and is under version control. |
| `pr100-hotstuff-v290-compact-execution-seq.py` | Run only after the HS1 tree exists and is under version control. |

## HS2 Detailed Audit

HS2 is a new protocol tree in the current fork. Direct application should begin
with the skeleton patch, then safety, then wiring.

| Patch | Representative targets | Status | Action |
| --- | --- | --- | --- |
| `hs2-fork-skeleton.patch` | `platform/consensus/ordering/hs2/*`, `benchmark/protocols/hs2/*`, `proto/hs2.proto` | mostly new tree required | First HS2 patch to apply after layout check. |
| `hs2-safety-prototype.patch` | `hs2_consensus.*`, `hs2_safety_probe.cpp` | new tree required | Apply after skeleton. |
| `hs2-resilientdb-wiring.patch` | `consensus_manager_hs2.*`, `hs2_wiring_probe.cpp` | new tree required | Apply after skeleton/safety. |
| `hs2-kv-service-build.patch` | `benchmark/protocols/hs2/kv_service.cpp` | new tree required | Apply after HS2 benchmark directory exists. |
| `hs2-client-request-runtime.patch` | `message_manager_basic.*`, `consensus_manager_hs2.*` | mixed/new path | Reconcile common framework path first. |
| `hs2-pacemaker-probe.patch` | `hs2_pacemaker.*`, probe | new tree required | Apply after core HS2 tree. |
| `hs2-leader-recovery-runtime.patch` | `response_manager.*`, `consensus_manager_hs2.*` | mixed/new path | Reconcile response manager path first. |
| `hs2-timeout-certificate-runtime.patch` | `hs2_timeout_certificate.*`, probe | new tree required | Apply after core HS2 tree. |
| `hs2-new-view-highqc-runtime.patch` | `hs2_new_view.*`, probe | new tree required | Apply after TC/highQC dependency review. |
| `hs2-negative-safety-tests.patch` | `hs2_negative_safety_probe.cpp` | new tree required | Apply near beta/conformance stage. |

## Recommended Application Order

### HS1 first

1. Recover/import PR100 `platform/consensus/ordering/hotstuff`.
2. Recover/import PR100 `benchmark/protocols/hotstuff`.
3. Verify Bazel can see HotStuff targets.
4. Apply KV benchmark/service patches.
5. Apply trace instrumentation.
6. Apply runtime repair patches through the current warm-cluster repair.
7. Instrument cold-start before changing NEWVIEW timing again.

### HS2 second

1. Apply HS2 skeleton.
2. Apply safety prototype.
3. Apply ResilientDB wiring.
4. Apply KV service build.
5. Reconcile common framework paths.
6. Apply client request runtime.
7. Apply pacemaker, leader recovery, TC, NewView/highQC.
8. Apply negative safety tests.

## Blockers Before Direct `git apply`

- `hotstuff` tree missing.
- `hs2` tree missing.
- common framework path drift.
- patch files intentionally preserve trailing whitespace from source exports to
  keep hashes stable.

Do not normalize or rewrite patch contents before applying; instead create new
fork-native commits once each group is reconciled.
