# v2.18.5-rc.1 - Runtime Phase Trace Extraction

## Objective

Extract observable runtime phase traces from the accepted local cold-start run:

```txt
documents/chatay-porting/v2.18.4-beta.1/logs/20260701T021524Z-runtime-coldstart
```

The goal is not to add new consensus behavior, but to preserve a structured
trace of what the current PBFT, HS1/PR100 and HS2 images actually emit during a
local no-fault KV run.

## Extractor

```bash
bash tools/chatay/benchmark/extract_runtime_phase_traces.sh
```

Generated artifacts:

```txt
phase-trace-events.csv
phase-trace-coverage.csv
PHASE_TRACE_COVERAGE.md
```

## Coverage Summary

| Protocol | Events extracted | Main observed protocol markers |
| --- | ---: | --- |
| PBFT | 753 | startup, readiness, heartbeat, PBFT commit, client SET/GET |
| HS1/PR100 | 1317 | startup, readiness, heartbeat, prepare/precommit/commit votes, client SET/GET |
| HS2 | 792 | startup, readiness, heartbeat, phase-2 certification, client SET/GET |

## Limitations

The accepted source run is a local no-fault run. Therefore, real view-change,
leader-failure and network-partition traces are not part of this gate. They
belong to the later heterogeneous/fault benchmark roadmap.
