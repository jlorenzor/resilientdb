# Chatay ResilientDB Fork Roadmap

## Scope

This fork is the consensus-layer research repository for Chatay. It keeps the
ResilientDB-side work in the same technical stack used by the upstream project:

```txt
ResilientDB
C++
Bazel
Docker
KV service path
```

Chatay itself remains responsible for the web/API application, workloads,
academic documentation, exported evidence, and benchmark orchestration.

## Branch namespace

Use professional branch names only:

```txt
consensus/<topic>-<semver>
```

Do not use personal or tool-specific prefixes.

## Versioned Roadmap

| SemVer | Status | Objective |
| --- | --- | --- |
| `v2.14.2-alpha.1` | Closed | Export current HS1/PR100 and HS2 patches from Chatay into this fork. |
| `v2.14.3-alpha.1` | Closed | Map the current ResilientDB structure for PBFT, HS1 and HS2. |
| `v2.14.4-alpha.1` | Closed | Apply HS1/PR100 as a reproducible branch/module inside the fork. |
| `v2.14.5-alpha.1` | Closed | Build HS1 in the fork with Bazel/Docker. |
| `v2.14.6-alpha.1` | Closed | Reproduce HS1 warm-cluster evidence from the fork. |
| `v2.14.7-alpha.1` | Closed | Instrument HS1 cold-start: key loading, NEWVIEW, readiness, replica init. |
| `v2.14.8-alpha.1` | Closed | Fix or parameterize HS1 NEWVIEW bootstrap. |
| `v2.14.9-beta.1` | Closed | Validate improved HS1 cold-start with repeated runs. |
| `v2.14.10-beta.1` | Closed | Freeze image `chatay-resilientdb-hs1-pr100:<semver>`. |
| `v2.14.11-rc.1` | Closed | HS1 conformance report: implementation vs original HotStuff. |
| `v2.15.0-alpha.1` | Closed | Start the clean HS2 port inside the fork after HS1 is closed. |
| `v2.15.1-alpha.1` | Closed | HS2 C++/Bazel skeleton. |
| `v2.15.2-alpha.1` | Closed | Block, vote, QC, TC, highQC, lockedQC. |
| `v2.15.3-alpha.1` | Closed | Safety rules and C++ probes. |
| `v2.15.4-alpha.1` | Closed | NewView/highQC and timeout certificate. |
| `v2.15.5-alpha.1` | Closed | HS2 integration with the KV path. |
| `v2.15.6-beta.1` | Closed | HS2 4-node smoke. |
| `v2.15.7-beta.1` | Closed | HS2 fault scenarios at probe/harness level. |
| `v2.15.8-rc.1` | Closed | HS2 conformance report. |
| `v2.16.0` | Closed | Stable PBFT vs HS1 vs HS2 baseline preparation milestone. |
| `v2.17.0-alpha.1` | Closed | Paper-to-code matrix for HS2 implementation gaps. |
| `v2.17.1-alpha.1` | Closed | Harden `TYPE_NEW_TXNS` so KV commit is gated by the experimental HS2 block/QC/safety pipeline. |
| `v2.17.2-alpha.1` | Closed | Reduce synthetic QC assumptions by separating vote collection, QC construction and signature/verifier boundaries. |
| `v2.17.3-alpha.1` | Closed | Bind payload digest, request hash and application execution evidence end-to-end. |
| `v2.17.4-alpha.1` | Closed | Add multi-process fault harnesses for stopped leader, stopped replica and slow startup. |
| `v2.17.5-beta.1` | Closed | Warm-cluster repeated runs and destructive process tests for HS2. |
| `v2.17.6-rc.1` | Closed | Updated HS2 conformance report after runtime hardening. |
| `v2.18.0-alpha.1` | Closed | Build and freeze a clean HS2 runtime image. |
| `v2.18.1-alpha.1` | Closed | Lock PBFT, HS1 and HS2 images for one comparable run. |
| `v2.18.2-alpha.1` | Closed | Implement segmented runner schema: image-check, build, cold-start, warm-cluster and phase trace. |
| `v2.18.3-alpha.1` | Closed | Normalize PBFT runtime image with the same executable layout used by HS1 and HS2. |
| `v2.18.3-beta.1` | Closed | Run local warm-cluster PBFT vs HS1 vs HS2 benchmark. |
| `v2.18.4-beta.1` | Closed | Run cold-start benchmark separated by protocol. |
| `v2.18.5-rc.1` | Closed | Extract real no-fault phase traces and mark view-change/fault traces as not executed. |
| `v2.18.6-rc.1` | Closed | Write local comparative report with explicit limits. |
| `v2.19.0` | Closed | Freeze stable local PBFT vs HS1 vs HS2 baseline. |

## Current Gate

The current local baseline is `v2.19.0`. It freezes the PBFT, HS1/PR100 and
HS2 local evidence chain for the frozen images:

```txt
PBFT      chatay-resilientdb-pbft:v2.18.3-alpha.1               ~1070 MB
HS1/PR100 chatay-resilientdb-hs1-pr100:v2.14.10-beta.1          ~976 MB
HS2       chatay-resilientdb-hs2:v2.18.0-alpha.1                ~976 MB
```

Older build/toolchain images remain excluded from benchmark timing. The
remaining benchmark segments stay explicitly separated as build, cold-start,
warm-cluster and phase-trace.

The preserved runtime contract remains:

```txt
4 consensus replicas
1 ResilientDB client/gateway process
KV SET
KV GET
returned value == written value
logs under documents/chatay-porting/<semver>/logs
```

The next work is deferred to a later version family and should focus on
heterogeneous-network preparation. Warm-cluster, cold-start, no-fault trace
extraction and the local comparative report are now closed for the frozen local
images, but they are not yet evidence for heterogeneous-network behavior.

## Claim Boundary

Do not claim HS1 as canonical HotStuff until `v2.14.11-rc.1` documents the
exact overlap and differences with the original HotStuff literature.

Do not claim HS2 as a complete production replacement for ResilientDB PBFT.
The correct current wording is:

```txt
The fork contains an experimental C++/Bazel HS2 path integrated with the
ResilientDB KV runtime. As of v2.18.0-alpha.1, the post-hardening HS2 runtime
has a differentiated lightweight Docker image with runtime binaries and labels.
Networked votes, cryptographic QC aggregation, network partitions, image-based
runtime smoke and full benchmark campaigns remain future gates.
```
