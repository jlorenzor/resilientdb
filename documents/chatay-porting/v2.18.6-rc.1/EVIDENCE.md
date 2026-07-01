# v2.18.6-rc.1 Evidence

## Closed Inputs

| Version | Artifact |
| --- | --- |
| `v2.18.3-beta.1` | `runtime-warm-summary.csv` |
| `v2.18.4-beta.1` | `cold-start-summary.csv` |
| `v2.18.5-rc.1` | `phase-trace-coverage.csv` |

## Main Evidence Files

```txt
LOCAL_COMPARATIVE_REPORT.md
local-comparative-summary.csv
```

## Accepted Interpretation

PBFT, HS1/PR100 and HS2 all completed the same local runtime KV path. The
largest local difference is HS1/PR100 readiness time, which is tracked as
startup behavior rather than steady-state throughput.

The local evidence is sufficient to freeze a local baseline in `v2.19.0`, but
not sufficient to claim heterogeneous-network performance.
