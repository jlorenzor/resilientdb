# v2.14.10-beta.1 - HS1 Runtime Image Freeze

## Objective

Freeze a differentiated HS1/PR100 runtime image for the current ResilientDB fork
state.

## Image

```txt
chatay-resilientdb-hs1-pr100:v2.14.10-beta.1
```

## Build Strategy

The runtime image is built from the previously size-validated HS1 runtime base,
but replaces the service/client binaries with artifacts compiled from the
current fork. This avoids reintroducing the full Bazel cache or source tree into
the runtime layer.

## Expected Size Policy

```txt
expectedRange=0.9GB-1.1GB
reviewThreshold=>1.25GB
```

## Claim Boundary

This version freezes a runtime image identity. It does not run the final
PBFT/HS1/HS2 benchmark.
