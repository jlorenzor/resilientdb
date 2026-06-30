# HS1 Bazel Reconciliation Checklist - v2.14.5-alpha.1

## Completed

- [x] Created branch `codex/hs1-bazel-reconcile-v2.14.5-alpha.1`.
- [x] Reused existing toolchain image `chatay-resilientdb-toolchain:bazel6-20260528`.
- [x] Fixed `.bazelversion` LF issue for Linux Docker.
- [x] Added `.gitattributes` rule to keep `.bazelversion` LF-only.
- [x] Confirmed `bazel query //platform/consensus/ordering/hotstuff:consensus`.
- [x] Confirmed `bazel query //benchmark/protocols/hotstuff:kv_service`.
- [x] Built `//platform/consensus/ordering/hotstuff:consensus`.
- [x] Fixed PR100 KV executor drift from `ChainState` to `MemoryDB`.
- [x] Built `//benchmark/protocols/hotstuff:kv_service`.
- [x] Built `//benchmark/protocols/hotstuff:kv_server_performance`.
- [x] Captured build logs under `documents/chatay-porting/v2.14.5-alpha.1/logs`.

## Deferred

- [ ] Produce a runtime image from this fork branch.
- [ ] Start a 4-node HS1 cluster from fork-built binaries.
- [ ] Execute KV `set/get` through HS1.
- [ ] Compare runtime behavior against the previous Chatay HS1 image.
- [ ] Instrument readiness and cold-start in the fork-native HS1 path.
