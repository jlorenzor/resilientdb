# Chatay Consensus Porting Package - v2.14.2-alpha.1

## Purpose

This package imports the current Chatay HS1/PR100 and HS2 research patch set
into the ResilientDB fork so the next work happens in the correct repository.

It is intentionally not a build or benchmark release.

## Scope

Included:

- HS1/PR100 patches used to make the HotStuff-like baseline executable and to
  diagnose/repair readiness behavior.
- HS2 patches used by the current C++/Bazel skeleton, safety probes, NewView,
  timeout certificate, KV path and negative safety checks.
- Manifest and image-size policy for later Docker work.

Excluded:

- Chatay API/Web code.
- Chatay workload runners and metrics exporters.
- GraphQL/Crow utility patches that are not part of consensus.
- Docker image builds.

## Repository Boundary

```txt
Chatay repository:
  application, workloads, metrics, report, benchmark orchestration.

resilientdb-hs2-fork:
  C++/Bazel consensus code, protocol patches, fork-local tests, Docker runtime
  image definitions.
```

## Base Commit

```txt
ce65a1ed7d3789d11fa8f154544d1bd31607fae5
```

Branch:

```txt
codex/hs2-consensus-port-v2.14.1-alpha.1
```

## Patch Groups

| Group | Directory | Count | Intent |
| --- | --- | ---: | --- |
| HS1/PR100 | `patches/hs1-pr100` | 13 | Port and harden the HotStuff-like baseline before HS2 benchmarking. |
| HS2 | `patches/hs2` | 10 | Recreate the current HS2 C++/Bazel alpha path inside this fork. |

## Why HS1 Is Included

HS1/PR100 is the bridge between upstream ResilientDB PBFT and the HS2 research
track. Leaving HS1 as Chatay-only patches would weaken the methodology because
HS2 would be fork-native while HS1 would remain an external artifact.

The HS1 cold-start issue is explicitly preserved for correction. Current
evidence points to bootstrap/readiness NEWVIEW behavior, not Docker build time
or image size.

## Image Size Policy

This package does not build images. Future runtime images must remain close to
the previously validated range:

```txt
target runtime image size: about 0.9 GB to 1.1 GB
hard review threshold: > 1.25 GB
```

Build toolchains, Bazel cache and source trees must stay out of runtime images.

See:

```txt
IMAGE_SIZE_POLICY.md
PATCH_MANIFEST.md
```

## Next Version

```txt
v2.14.3-alpha.1 - ResilientDB Layout Map for PBFT, HS1 and HS2
```

The next version should map the current ResilientDB source tree before applying
patches, because path names differ from older PR documentation.
