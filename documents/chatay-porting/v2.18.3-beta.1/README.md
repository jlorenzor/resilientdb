# v2.18.3-beta.1 - Local Runtime Warm-Cluster Baseline

## Objective

Run PBFT, HS1/PR100 and HS2 with frozen Docker runtime images and validate that
each protocol can complete the same local KV path:

```txt
4 consensus replicas
1 ResilientDB client/gateway process
KV SET
KV GET
returned value == written value
```

This gate intentionally excludes build time. It measures local runtime behavior
after images already exist.

## Runtime Images

| Protocol | Image |
| --- | --- |
| PBFT | `chatay-resilientdb-pbft:v2.18.3-alpha.1` |
| HS1/PR100 | `chatay-resilientdb-hs1-pr100:v2.14.10-beta.1` |
| HS2 | `chatay-resilientdb-hs2:v2.18.0-alpha.1` |

## Runner

The runner added in this version is:

```bash
tools/chatay/benchmark/run_runtime_image_kv_cluster.sh
```

It creates a short-lived Docker container for each protocol image, copies a
small execution payload into that container, generates local keys/certificates,
writes ResilientDB-compatible JSON configs, starts the five KV processes, runs
SET/GET operations, copies logs back to the host, and removes the container.

## Evidence Run

The closed run used:

```bash
CHATAY_PROTOCOLS=pbft,hs1-pr100,hs2 \
CHATAY_OPERATION_COUNT=10 \
bash tools/chatay/benchmark/run_runtime_image_kv_cluster.sh
```

Run ID:

```txt
20260701T020837Z-runtime-warm
```

All three protocols passed 10/10 operations in the local runtime warm-cluster
gate.

## Claim Boundary

This is a local runtime warm-cluster validation. It is not a heterogeneous
network benchmark, not a throughput campaign, and not final experimental
evidence for national-scale deployment. It is the baseline gate required before
separating cold-start and phase-trace measurements.
