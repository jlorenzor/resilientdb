# v2.18.4-beta.1 - Local Runtime Cold-Start Matrix

## Objective

Measure local cold-start behavior for PBFT, HS1/PR100 and HS2 from frozen
runtime images, while separating:

```txt
containerTotalDurationMs
processReadyDurationMs
operationsDurationMs
```

This gate still runs locally in Docker Desktop. It does not represent
heterogeneous-device performance.

## Command

```bash
CHATAY_PROTOCOLS=pbft,hs1-pr100,hs2 \
CHATAY_REPEAT_COUNT=3 \
bash tools/chatay/benchmark/run_runtime_image_coldstart_matrix.sh
```

Run ID:

```txt
20260701T021524Z-runtime-coldstart
```

## Methodological Fix

An initial run used service ports above `32768`, inside the common Linux
ephemeral port range. One HS2 node hit a transient `Address already in use`
during startup. The runner was corrected to use `25xxx` service ports, outside
the common ephemeral range, and the full 9/9 matrix passed.

This is a benchmark-harness correction, not a consensus change.

## Result

All three protocols passed three cold-start repetitions with one SET/GET pair
per repetition.
