# Image Lock Decision

## Decision

Use the following protocol images as the only comparable runtime set for the
next benchmark gates:

```txt
PBFT      chatay-resilientdb-pbft:runtime-v2.13.20-alpha.1
HS1/PR100 chatay-resilientdb-hs1-pr100:v2.14.10-beta.1
HS2       chatay-resilientdb-hs2:v2.18.0-alpha.1
```

## Reason

Earlier images include build tools, Bazel caches, source trees or historical
experiments. They are useful for reproducibility and repair, but they distort
runtime-only comparisons.

The locked images are all runtime-oriented and keep the expected image footprint
around the 0.9 GB to 1.1 GB policy range established for Chatay consensus
experiments.

## Benchmark Consequence

`v2.18.2-alpha.1` must consume these image names explicitly. The runner must not
fall back to floating tags, the toolchain image, or historical 8 GB images.

## Scope Limit

This decision does not remove local Docker images. It only defines which images
are valid for comparative PBFT vs HS1 vs HS2 runtime measurements.
