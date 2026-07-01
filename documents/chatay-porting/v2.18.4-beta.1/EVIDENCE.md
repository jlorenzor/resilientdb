# v2.18.4-beta.1 Evidence

## Final Matrix

| Protocol | Repeat | Status | Container total ms | Process ready ms | Ops ms | Passed |
| --- | ---: | --- | ---: | ---: | ---: | ---: |
| PBFT | 1 | `PASSED` | 5585 | 12 | 1264 | 1/1 |
| PBFT | 2 | `PASSED` | 5519 | 9 | 1201 | 1/1 |
| PBFT | 3 | `PASSED` | 5516 | 8 | 1201 | 1/1 |
| HS1/PR100 | 1 | `PASSED` | 18821 | 13099 | 1320 | 1/1 |
| HS1/PR100 | 2 | `PASSED` | 18745 | 13098 | 1322 | 1/1 |
| HS1/PR100 | 3 | `PASSED` | 18917 | 13095 | 1364 | 1/1 |
| HS2 | 1 | `PASSED` | 5565 | 12 | 1180 | 1/1 |
| HS2 | 2 | `PASSED` | 5663 | 6 | 1220 | 1/1 |
| HS2 | 3 | `PASSED` | 5283 | 6 | 1176 | 1/1 |

## Interpretation

HS1/PR100 consistently spends around 13 seconds in process readiness on this
local runner. PBFT and HS2 readiness are much shorter in the same harness.

This does not yet prove global performance superiority. It identifies a startup
behavior that must be separated from steady-state operation in the thesis
methodology.

## Rejected Run

The earlier run `20260701T021253Z-runtime-coldstart` is kept only in ignored
logs. It failed one HS2 repetition because the benchmark harness used ports in
the common ephemeral range. The final accepted run is
`20260701T021524Z-runtime-coldstart`.
