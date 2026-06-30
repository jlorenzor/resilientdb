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
| `v2.17.4-alpha.1` | Next | Add multi-process fault harnesses for stopped leader, stopped replica and slow startup. |
| `v2.17.5-beta.1` | Pending | Warm-cluster repeated runs and destructive process tests for HS2. |
| `v2.17.6-rc.1` | Pending | Updated HS2 conformance report after runtime hardening. |
| `v2.18.0` | Pending | Benchmark pre-release candidate for PBFT vs HS1 vs HS2. |

## Current Gate

The current gate is `v2.17.4-alpha.1`. `v2.17.3-alpha.1` proved that
request payload, request hash, QC proof material and pre-execution evidence can
be bound into the alpha commit proof while preserving:

```txt
4 consensus replicas
1 ResilientDB client/gateway process
KV SET
KV GET
returned value == written value
logs under documents/chatay-porting/v2.17.1-alpha.1/logs
```

Benchmarking remains blocked until the HS2 runtime path reduces synthetic QC
assumptions and records repeatable warm-cluster behavior.

## Claim Boundary

Do not claim HS1 as canonical HotStuff until `v2.14.11-rc.1` documents the
exact overlap and differences with the original HotStuff literature.

Do not claim HS2 as a complete production replacement for ResilientDB PBFT.
The correct current wording is:

```txt
The fork contains an experimental C++/Bazel HS2 path integrated with the
ResilientDB KV runtime. As of v2.17.1-alpha.1, TYPE_NEW_TXNS is gated by a local
HS2 block/QC/safety pipeline before commit, but networked votes, cryptographic
QC aggregation and destructive fault campaigns remain future gates.
```
