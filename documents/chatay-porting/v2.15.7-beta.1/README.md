# v2.15.7-beta.1 - HS2 Fault Probes

## Status

Closed as probe-level validation.

## Covered Fault Cases

| Scenario | Evidence target | Result |
|---|---|---|
| Stopped or missing replica vote | `hs2_negative_safety_probe` | Insufficient QC/TC rejected. |
| Duplicate or equivocated vote | `hs2_negative_safety_probe` | Duplicate and conflicting votes rejected. |
| Leader stopped / next leader needed | `hs2_pacemaker_probe`, `hs2_timeout_certificate_probe` | View advances and next leader is selected. |
| Slow replica | `hs2_timeout_certificate_probe` | Quorum forms without all replicas. |
| Unsafe proposal after lock | `hs2_negative_safety_probe` | Unsafe proposal rejected. |

## Probe Output

```txt
HS2_NEGATIVE_SAFETY_PASSED cases=13 duplicate_qc=reject insufficient_qc=reject conflicting_vote=reject insufficient_tc=reject duplicate_tc=reject invalid_new_view=reject unsafe_proposal=reject leader_advance=ok
```

## Boundary

This is fault-path validation of the HS2 state machine, not a destructive
multi-process BFT fault campaign. The latter remains part of the benchmark
phase after the protocol implementation is hardened.
