# v2.19.0 - Stable Local Consensus Baseline

## Objective

Freeze the local PBFT vs HS1/PR100 vs HS2 baseline after the runtime image,
warm-cluster, cold-start, phase-trace and comparative-report gates.

This is the stable local checkpoint before moving to heterogeneous-network
preparation in later versions.

## Frozen Inputs

| Gate | Status | Evidence |
| --- | --- | --- |
| `v2.18.3-alpha.1` | Closed | PBFT runtime image normalized |
| `v2.18.3-beta.1` | Closed | Local runtime warm-cluster |
| `v2.18.4-beta.1` | Closed | Local runtime cold-start matrix |
| `v2.18.5-rc.1` | Closed | Local no-fault phase traces |
| `v2.18.6-rc.1` | Closed | Local comparative report |

## Frozen Runtime Images

| Protocol | Image | Image ID | Size |
| --- | --- | --- | ---: |
| PBFT | `chatay-resilientdb-pbft:v2.18.3-alpha.1` | `sha256:47c138d18e5061c6eb7461e876f2cc8bbd6ca8af29bf6ed18376732da059aa4a` | 1070001534 bytes |
| HS1/PR100 | `chatay-resilientdb-hs1-pr100:v2.14.10-beta.1` | `sha256:685173b06c9274d1e6a63327143001a030915853d10daa5fbaf843b3bc2a5717` | 976420827 bytes |
| HS2 | `chatay-resilientdb-hs2:v2.18.0-alpha.1` | `sha256:87d206e1a2ad9bc0d7af662479fa45e77da5f0c82238b67249450a6576bd1224` | 976114081 bytes |

## Accepted Baseline Claim

PBFT, HS1/PR100 and HS2 are runnable from frozen Docker runtime images through
the same local ResilientDB KV path with four consensus replicas and one
client/gateway process.

## Claim Boundary

This stable baseline is local. It does not include heterogeneous devices,
network partitions, leader failure, Byzantine behavior, long-running throughput
campaigns, or statistical confidence intervals.
