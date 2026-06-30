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
| `v2.14.6-alpha.1` | In progress | Reproduce HS1 warm-cluster `30/30` from the fork. |
| `v2.14.7-alpha.1` | Pending | Instrument HS1 cold-start: key loading, NEWVIEW, readiness, replica init. |
| `v2.14.8-alpha.1` | Pending | Fix or parameterize HS1 NEWVIEW bootstrap. |
| `v2.14.9-beta.1` | Pending | Validate improved HS1 cold-start with repeated runs. |
| `v2.14.10-beta.1` | Pending | Freeze image `chatay-resilientdb-hs1-pr100:<semver>`. |
| `v2.14.11-rc.1` | Pending | HS1 conformance report: implementation vs original HotStuff. |
| `v2.15.0-alpha.1` | Pending | Start the clean HS2 port inside the fork after HS1 is closed. |
| `v2.15.1-alpha.1` | Pending | HS2 C++/Bazel skeleton. |
| `v2.15.2-alpha.1` | Pending | Block, vote, QC, TC, highQC, lockedQC. |
| `v2.15.3-alpha.1` | Pending | Safety rules and C++ probes. |
| `v2.15.4-alpha.1` | Pending | NewView/highQC and timeout certificate. |
| `v2.15.5-alpha.1` | Pending | HS2 integration with the KV path. |
| `v2.15.6-beta.1` | Pending | HS2 4-node smoke. |
| `v2.15.7-beta.1` | Pending | HS2 fault scenarios: stopped node, stopped leader, slow replica. |
| `v2.15.8-rc.1` | Pending | HS2 conformance report. |
| `v2.16.0` | Pending | Stable PBFT vs HS1 vs HS2 baseline ready for comparative benchmark. |

## Current Gate

The current gate is `v2.14.6-alpha.1`. It must prove that HS1/PR100 can run
from fork-built binaries with:

```txt
4 consensus replicas
1 ResilientDB client/gateway process
KV SET
KV GET
returned value == written value
logs and manifest under documents/chatay-porting/v2.14.6-alpha.1/logs
```

Benchmarking remains blocked until the functional gate is closed.

## Claim Boundary

Do not claim HS1 as canonical HotStuff until `v2.14.11-rc.1` documents the
exact overlap and differences with the original HotStuff literature.

Do not claim HS2 as implemented inside ResilientDB until `v2.15.x` produces
fork-native C++/Bazel code and passes the KV and fault gates.
