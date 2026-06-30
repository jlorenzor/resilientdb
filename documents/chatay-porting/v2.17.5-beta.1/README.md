# v2.17.5-beta.1 - HS2 Repeatability Gate

## Objective

This milestone adds a repeatability gate for the experimental HS2 KV runtime.
It exercises the same four-replica warm-cluster path several times with larger
operation counts before the work is promoted to a release-candidate conformance
report.

The gate is intentionally scoped to runtime continuity:

```txt
4 consensus replicas
1 ResilientDB KV client/gateway process
SET operations
GET operations
returned value == written value
HS2 commit proof attached before KV commit
```

## Added Runner

```txt
tools/chatay/hs2/run_hs2_repeatability_gate.sh
```

The runner wraps `run_hs2_kv_smoke_cluster.sh` and executes a configurable list
of operation counts. By default it uses:

```txt
HS2_REPEAT_OPERATION_COUNTS=30,100
HS2_AFTER_SET_SLEEP_SEC=0.2
```

The short after-SET sleep keeps the local warm-cluster from being dominated by
the earlier fixed one-second delay used during smoke validation.

## Validated Evidence

The local Docker toolchain run completed with:

```txt
30/30 operations passed
100/100 operations passed
```

The generated CSV summary is:

```txt
documents/chatay-porting/v2.17.5-beta.1/logs/20260630T-v2175-hs2-repeat/summary.csv
```

Only `logs/.gitignore` is versioned. Raw run logs remain local because they can
be large and environment-specific.

## Claim Boundary

This version does not claim HS2 is production-ready, nor that it outperforms
PBFT or HS1. It only establishes that the hardened HS2 path can complete
repeated warm-cluster KV workloads under the local Docker/Bazel environment.

Comparative benchmarking remains a later gate after the PBFT, HS1 and HS2
runtime images are frozen and the benchmark harness separates build time,
cold-start time and warm-cluster operation time.
