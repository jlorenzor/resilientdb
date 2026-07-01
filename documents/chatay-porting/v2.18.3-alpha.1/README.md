# v2.18.3-alpha.1 - PBFT Runtime Normalization

## Status

Alpha prerequisite for warm-cluster benchmarking.

## Purpose

The locked PBFT image from `v2.18.1-alpha.1` identified PBFT as a runtime image,
but it did not expose the same `/opt/resilientdb-*/bin` runtime layout used by
HS1 and HS2.

This gate creates a normalized PBFT runtime image before any PBFT vs HS1 vs HS2
benchmark is attempted. It uses a lightweight benchmark target,
`//benchmark/protocols/pbft:kv_service`, instead of the product KV target that
pulls DuckDB into the build.

## Expected Image

```txt
chatay-resilientdb-pbft:v2.18.3-alpha.1
```

## Claim Boundary

This milestone is packaging normalization only. It is not a benchmark result.
