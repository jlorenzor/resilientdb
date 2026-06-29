# v2.14.4-alpha.1 - HS1/PR100 Baseline Tree Port

## Objective

Recover the HotStuff/HS1 baseline tree from Apache ResilientDB PR100 into the
current `jlorenzor/resilientdb` fork, without claiming runtime correctness yet.

This block exists because `v2.14.3-alpha.1` confirmed that current upstream has
PBFT, GeoPBFT and PoE, but no `hotstuff` or `hs2` protocol trees.

## Source

Reference imported locally:

```txt
upstream/pr/100
```

Fetched from:

```txt
https://github.com/apache/incubator-resilientdb pull/100/head
```

PR100 commit observed locally:

```txt
44ce20aa add hs
```

## Imported protocol tree

```txt
platform/consensus/ordering/hotstuff/BUILD
platform/consensus/ordering/hotstuff/commitment.cpp
platform/consensus/ordering/hotstuff/commitment.h
platform/consensus/ordering/hotstuff/consensus.cpp
platform/consensus/ordering/hotstuff/consensus.h
platform/consensus/ordering/hotstuff/message_manager.cpp
platform/consensus/ordering/hotstuff/message_manager.h
platform/consensus/ordering/hotstuff/proto/BUILD
platform/consensus/ordering/hotstuff/proto/hotstuff.proto
```

## Imported common dependencies

PR100 HotStuff depends on common ordering helpers that are not present in the
current upstream root `common` package. These were restored as part of the HS1
baseline:

```txt
platform/consensus/ordering/common/atomic_unique_ptr.h
platform/consensus/ordering/common/commitment_basic.cpp
platform/consensus/ordering/common/commitment_basic.h
platform/consensus/ordering/common/message_manager_basic.cpp
platform/consensus/ordering/common/message_manager_basic.h
platform/consensus/ordering/common/performance_manager.cpp
platform/consensus/ordering/common/performance_manager.h
platform/consensus/ordering/common/response_manager.cpp
platform/consensus/ordering/common/response_manager.h
```

The existing root `transaction_utils.*` files were preserved from current
upstream. They were not replaced with the PR100 versions.

## Added benchmark/KV targets

These files were restored from the preserved Chatay patch package exported in
`v2.14.2-alpha.1`:

```txt
benchmark/protocols/hotstuff/BUILD
benchmark/protocols/hotstuff/kv_server_performance.cpp
benchmark/protocols/hotstuff/kv_service.cpp
```

## BUILD wiring added

The current `platform/consensus/ordering/common/BUILD` did not expose PR100
common targets. This block adds labels for:

```txt
//platform/consensus/ordering/common:atomic_unique_ptr
//platform/consensus/ordering/common:message_manager_basic
//platform/consensus/ordering/common:commitment_basic
//platform/consensus/ordering/common:response_manager
//platform/consensus/ordering/common:performance_manager
```

## Static validation

Host Bazel was not available in the Windows PATH during this block, so no real
`bazel query` or build is claimed here.

A static label check confirmed that the following labels now have BUILD files
and named targets:

```txt
//platform/consensus/ordering/hotstuff:consensus
//platform/consensus/ordering/hotstuff:commitment
//platform/consensus/ordering/hotstuff:message_manager
//benchmark/protocols/hotstuff:kv_service
//benchmark/protocols/hotstuff:kv_server_performance
//platform/consensus/ordering/common:commitment_basic
//platform/consensus/ordering/common:message_manager_basic
//platform/consensus/ordering/common:response_manager
//platform/consensus/ordering/common:performance_manager
```

## What this version proves

This version proves that the HS1/PR100 source tree is now present in the real
ResilientDB fork, together with the benchmark/KV entrypoints needed for later
cluster execution.

## What this version does not prove

This version does not prove that HS1 builds or runs yet.

Build, API reconciliation and runtime validation are intentionally left for the
next blocks:

```txt
v2.14.5-alpha.1 - HS1 Bazel Query and Build Reconciliation
v2.14.6-alpha.1 - HS1 KV Warm-Cluster Runtime Reproduction
```

## Known reconciliation risks

1. PR100 was written against an older ResilientDB layout.
2. Current upstream also has `platform/consensus/ordering/common/framework/*`.
3. Root `common` and `common/framework` now contain similarly named response and
   performance manager files.
4. Further build errors may expose API drift in `SystemInfo`, `ConsensusManager`,
   `ReplicaCommunicator`, `SignatureVerifier`, or executor interfaces.
5. Cold-start/readiness behavior is not addressed here; it remains a later
   runtime task.

## Docker policy

No Docker image was built in this block.

Runtime image policy remains:

```txt
expected runtime image size: 0.9 GB - 1.1 GB
review threshold: > 1.25 GB
```
