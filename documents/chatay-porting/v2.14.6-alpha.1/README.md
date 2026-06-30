# v2.14.6-alpha.1 - HS1 KV Runtime Reproduction

## Objective

Run HS1/PR100 from fork-built binaries in a 4-node local cluster and validate a
KV `set/get` path.

## Starting point

Branch:

```txt
consensus/hs1-bazel-reconcile-v2.14.5-alpha.1
```

Known passing build targets:

```txt
//platform/consensus/ordering/hotstuff:consensus
//benchmark/protocols/hotstuff:kv_service
//benchmark/protocols/hotstuff:kv_server_performance
```

## Recommended next branch

```txt
consensus/hs1-kv-runtime-v2.14.6-alpha.1
```

## First commands to retry

```bash
docker run --rm \
  -v "<fork>:/workspace" \
  -v chatay-bazel-cache:/root/.cache/bazel \
  -w /workspace \
  chatay-resilientdb-toolchain:bazel6-20260528 \
  bazel build --jobs=4 //benchmark/protocols/hotstuff:kv_service
```

Then inspect:

```txt
bazel-bin/benchmark/protocols/hotstuff/kv_service
```

## Runtime checklist

- [ ] Create branch `consensus/hs1-kv-runtime-v2.14.6-alpha.1`.
- [ ] Locate existing config/key generation scripts.
- [ ] Start 4 HS1 nodes with `kv_service`.
- [ ] Capture one log file per node.
- [ ] Add readiness timestamps.
- [ ] Send KV `SET`.
- [ ] Send KV `GET`.
- [ ] Verify returned value.
- [ ] Save evidence under `documents/chatay-porting/v2.14.6-alpha.1/logs`.
- [ ] If runtime fails, classify as config/key/network/readiness/consensus.

## Claim boundary

Do not benchmark HS1 yet. This block is only a functional runtime gate.
