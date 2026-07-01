# v2.19.0 Evidence

## Baseline Evidence Chain

| Version | Evidence |
| --- | --- |
| `v2.18.3-beta.1` | `documents/chatay-porting/v2.18.3-beta.1/runtime-warm-summary.csv` |
| `v2.18.4-beta.1` | `documents/chatay-porting/v2.18.4-beta.1/cold-start-summary.csv` |
| `v2.18.5-rc.1` | `documents/chatay-porting/v2.18.5-rc.1/PHASE_TRACE_COVERAGE.md` |
| `v2.18.6-rc.1` | `documents/chatay-porting/v2.18.6-rc.1/LOCAL_COMPARATIVE_REPORT.md` |

## Stable Local Result

The local baseline validates that PBFT, HS1/PR100 and HS2 complete the same KV
SET/GET path from frozen runtime images.

## Known Local Finding

HS1/PR100 has a consistent local readiness delay of about 13 seconds in this
harness. This is recorded as startup/readiness behavior and must not be mixed
with steady-state throughput interpretation.
