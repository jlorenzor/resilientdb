# v2.14.5-alpha.1 - HS1 Bazel Query and Build Reconciliation

## Objective

Validate that the HS1/PR100 baseline tree imported in `v2.14.4-alpha.1` is
visible to Bazel and can build inside the existing ResilientDB toolchain image.

## Branch

```txt
consensus/hs1-bazel-reconcile-v2.14.5-alpha.1
```

## Toolchain

Docker image used:

```txt
chatay-resilientdb-toolchain:bazel6-20260528
```

Bazel version:

```txt
bazel 6.0.0
```

## Fixes applied

### `.bazelversion`

The Windows checkout had `.bazelversion` with CRLF and an extra blank line.
Inside Linux Docker, the wrapper interpreted it as `6.0.0\r`.

Fix:

```txt
.bazelversion now contains a single LF-terminated line: 6.0.0
.gitattributes pins .bazelversion as text eol=lf
```

### HS1 KV executor storage

PR100 used:

```cpp
std::make_unique<KVExecutor>(std::make_unique<ChainState>())
```

Current ResilientDB expects:

```cpp
std::make_unique<KVExecutor>(std::make_unique<MemoryDB>())
```

The HotStuff benchmark and KV entrypoints were updated to match the current
PBFT/PoE pattern.

## Commands executed

```bash
docker run --rm \
  -v "<fork>:/workspace" \
  -w /workspace \
  chatay-resilientdb-toolchain:bazel6-20260528 \
  bazel query //platform/consensus/ordering/hotstuff:consensus

docker run --rm \
  -v "<fork>:/workspace" \
  -w /workspace \
  chatay-resilientdb-toolchain:bazel6-20260528 \
  bazel query //benchmark/protocols/hotstuff:kv_service
```

Builds:

```bash
bazel build --jobs=4 //platform/consensus/ordering/hotstuff:consensus
bazel build --jobs=4 //benchmark/protocols/hotstuff:kv_service
bazel build --jobs=4 //benchmark/protocols/hotstuff:kv_server_performance
```

## Results

| Target | Result | Evidence |
| --- | --- | --- |
| `//platform/consensus/ordering/hotstuff:consensus` | PASS | `logs/hotstuff-consensus-build.log` |
| `//benchmark/protocols/hotstuff:kv_service` | PASS | `logs/hotstuff-kv-service-build.log` |
| `//benchmark/protocols/hotstuff:kv_server_performance` | PASS | `logs/hotstuff-kv-server-performance-build.log` |

Evidence excerpts:

```txt
Target //platform/consensus/ordering/hotstuff:consensus up-to-date
INFO: Build completed successfully, 7 total actions

Target //benchmark/protocols/hotstuff:kv_service up-to-date
INFO: Build completed successfully, 5 total actions

Target //benchmark/protocols/hotstuff:kv_server_performance up-to-date
INFO: Build completed successfully, 392 total actions
```

## Important operational note

Do not run multiple Bazel builds in parallel against the same Docker volume.
During this block, parallel use of the same cache volume produced a transient
`text file busy` error in Bazel's embedded JDK. The successful validation was
performed sequentially.

## What this version proves

HS1/PR100 now has a Bazel-buildable consensus target and buildable KV/benchmark
entrypoints inside the real ResilientDB fork.

## What this version does not prove

This version does not prove runtime cluster correctness. It does not start four
replicas, issue KV `set/get`, or evaluate cold-start/readiness.

Those tasks belong to:

```txt
v2.14.6-alpha.1 - HS1 KV Warm-Cluster Runtime Reproduction
v2.14.7-alpha.1 - HS1 Cold-Start and Readiness Instrumentation
```

## Docker image policy

No new Docker image was built in this version.

The existing toolchain image was reused. Runtime image limits remain:

```txt
expected runtime image size: 0.9 GB - 1.1 GB
review threshold: > 1.25 GB
```
