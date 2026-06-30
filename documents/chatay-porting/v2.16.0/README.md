# v2.16.0 - Baseline Ready: PBFT vs HS1 vs HS2

## Status

Closed as a baseline-preparation milestone.

## Protocol Candidates

| Protocol | Current fork status | Benchmark readiness |
|---|---|---|
| PBFT | Existing ResilientDB baseline image exists as `chatay-resilientdb-pbft:runtime-v2.13.20-alpha.1` | Ready for warm/cold/e2e segmentation. |
| HS1/PR100 | Warm-cluster, cold-start, image freeze and conformance report closed through `v2.14.11-rc.1` | Ready as intermediate HotStuff-like baseline. |
| HS2 | C++/Bazel module, probes, KV build and local smoke closed through `v2.15.8-rc.1` | Ready as experimental candidate with stated limitations. |

## Required Benchmark Segmentation

The comparative benchmark must report at least three timing layers:

1. Build/release preparation time: excluded from runtime performance claims.
2. Cold-start time: process startup, config, keys, service network and first
   readiness.
3. Warm-cluster operation latency: SET/GET or transaction path after nodes are
   already alive.

This segmentation is mandatory because cold-start artifacts can hide the real
consensus/runtime behavior.

## Current Baseline Decision

`v2.16.0` does not publish final comparative results. It freezes the fact that
all three protocol candidates have a defined experimental path and can enter the
next benchmark stage.

## Open Work After v2.16.0

- Build differentiated runtime images for HS2 with the same size policy used by
  PBFT and HS1.
- Run repeated warm-cluster trials for PBFT, HS1 and HS2.
- Add destructive process/network fault campaigns after HS2 message-pipeline
  hardening.
- Export comparable CSV metrics for the thesis results chapter.
