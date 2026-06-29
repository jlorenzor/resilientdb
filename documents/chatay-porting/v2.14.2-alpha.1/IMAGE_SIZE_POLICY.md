# Runtime Image Size Policy - v2.14.2-alpha.1

## Decision

Consensus images must stay distinguishable by protocol and close to the runtime
image sizes already validated in Chatay.

## Current Constraint

The runtime images previously validated were around 0.9 GB to 1.1 GB. Future
fork-produced images must preserve that order of magnitude.

```txt
Expected range: 0.9 GB - 1.1 GB
Review threshold: > 1.25 GB
Rejected by default: images that include Bazel cache, compiler toolchain or full
source tree in the runtime layer.
```

## Required Image Families

| Protocol | Image family |
| --- | --- |
| PBFT | `chatay-resilientdb-pbft:<semver>` |
| HS1/PR100 | `chatay-resilientdb-hs1-pr100:<semver>` |
| HS2 | `chatay-resilientdb-hs2:<semver>` |

## Rules

1. Builder stage may contain Bazel, compilers and full source.
2. Runtime stage must copy only binaries, configs, certificates, runtime
   libraries and small scripts needed to start the service.
3. Bazel cache must not be copied into the runtime image.
4. Source tree must not be copied into the runtime image unless a specific
   runtime dependency proves it is required.
5. Each image must include labels for protocol, source commit and Chatay
   research semver.
6. Build time must not be mixed with protocol performance metrics.

## Acceptance Gate For Future Docker Versions

Before an image is accepted:

- `docker image inspect` records byte size and labels.
- runtime image size is compared against the threshold above.
- a smoke or warm-cluster runner proves the image is executable.
- Chatay records the image in its consensus image lock.

## Claim Boundary

This file is a policy document. It does not create or optimize images yet.
