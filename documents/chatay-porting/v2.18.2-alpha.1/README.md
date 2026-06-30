# v2.18.2-alpha.1 - Segmented Benchmark Runner

## Status

Alpha runner framework.

## Purpose

Create a common benchmark output schema before running PBFT vs HS1 vs HS2
comparisons. The runner separates:

```txt
image-check
build
cold-start
warm-cluster
phase-trace
```

## Command

```bash
bash tools/chatay/benchmark/run_segmented_protocol_benchmark.sh
```

The default alpha behavior executes only `image-check` and records all other
segments as planned placeholders. This prevents premature performance claims.

## Outputs

```txt
documents/chatay-porting/v2.18.2-alpha.1/logs/<run-id>/segmented-summary.csv
documents/chatay-porting/v2.18.2-alpha.1/logs/<run-id>/segmented-manifest.json
documents/chatay-porting/v2.18.2-alpha.1/segmented-runner-plan.md
```

## Claim Boundary

This milestone proves a shared measurement schema and image availability gate.
It does not yet benchmark PBFT, HS1 or HS2.
