# v2.18.2-alpha.1 Evidence

## Command

```bash
bash tools/chatay/benchmark/run_segmented_protocol_benchmark.sh
```

## Expected Alpha Result

The runner must:

```txt
PASS image-check for PBFT, HS1 and HS2
record build as PLANNED
record cold-start as PLANNED
record warm-cluster as PLANNED
record phase-trace as PLANNED
write segmented-summary.csv
write segmented-manifest.json
write segmented-runner-plan.md
```

## Observed Run

```txt
runId=20260630T235818Z-segmented-runner
image-check pbft PASSED durationMs=214 sizeBytes=1011062608
image-check hs1-pr100 PASSED durationMs=216 sizeBytes=976420827
image-check hs2 PASSED durationMs=180 sizeBytes=976114081
build/cold-start/warm-cluster/phase-trace PLANNED for all protocols
```

## Interpretation

The output is not a benchmark. It is the measurement contract for the next
version gates.
