# Local Comparative Report - v2.18.6-rc.1

## Scope

This report compares PBFT, HS1/PR100 and HS2 in a local Docker Desktop runtime
environment. The purpose is to validate that the three frozen images can be run
through the same ResilientDB KV path and to identify measurement dimensions that
must be separated in the thesis methodology.

The evaluated path is:

```txt
runtime image -> 4 consensus replicas -> 1 client/gateway process -> SET -> GET
```

## Inputs

| Source | Evidence |
| --- | --- |
| `v2.18.3-beta.1` | Runtime warm-cluster with 10 SET/GET operations per protocol |
| `v2.18.4-beta.1` | Runtime cold-start matrix with 3 repetitions per protocol |
| `v2.18.5-rc.1` | Phase-trace extraction from the accepted cold-start run |

## Warm-Cluster Result

| Protocol | Operations | Passed | Total ms | Ready ms | Operations ms |
| --- | ---: | ---: | ---: | ---: | ---: |
| PBFT | 10 | 10 | 17307 | 9 | 12040 |
| HS1/PR100 | 10 | 10 | 31382 | 13110 | 12722 |
| HS2 | 10 | 10 | 17048 | 14 | 11621 |

In this local run, all protocols completed the same KV path. HS1/PR100 spent
substantially more time reaching readiness. PBFT and HS2 had similar local
runtime totals in this single warm-cluster run.

## Cold-Start Result

| Protocol | Repeats | Container avg ms | Ready avg ms | Operations avg ms |
| --- | ---: | ---: | ---: | ---: |
| PBFT | 3 | 5540 | 9.67 | 1222 |
| HS1/PR100 | 3 | 18827.67 | 13097.33 | 1335.33 |
| HS2 | 3 | 5503.67 | 8 | 1192 |

The cold-start matrix confirms that HS1/PR100 readiness is the dominant local
difference in this harness. This finding should be treated as startup behavior,
not as steady-state consensus throughput.

## Phase-Trace Coverage

| Protocol | Trace events | Observed categories | Not executed categories |
| --- | ---: | ---: | ---: |
| PBFT | 753 | 9 | 1 |
| HS1/PR100 | 1317 | 12 | 1 |
| HS2 | 792 | 9 | 1 |

PBFT emitted commit markers. HS1/PR100 emitted prepare, precommit and commit
vote markers plus consensus commit markers. HS2 emitted `CHATAY_HS2_PIPELINE
certified` markers with `phase2_voters=3`. All three emitted client-visible
SET/GET success markers.

No view-change or leader-failure trace was executed in this gate.

## Methodological Notes

Build time is excluded. These tests use already-built runtime images.

Container total time includes Docker create/start/exec/copy/remove overhead.
Process readiness time is measured inside the container after the KV processes
start. Operation time covers the client SET/GET path.

The cold-start runner uses service ports below the common Linux ephemeral range
to avoid transient bind conflicts during fast local startup.

## Interpretation

The local baseline supports three claims:

1. PBFT, HS1/PR100 and HS2 can run the same local ResilientDB KV path from
   frozen Docker runtime images.
2. HS1/PR100 has a reproducible local readiness delay that must be separated
   from steady-state operation in the methodology.
3. HS2 is integrated deeply enough to produce runtime certification traces and
   complete the KV SET/GET path, but this does not yet prove production-grade
   HotStuff-2 conformance.

## Limits

This report does not include heterogeneous devices, network partitions, leader
failure, replica failure, Byzantine behavior, long-running throughput tests, or
statistical confidence intervals. Those belong to later benchmark gates.
