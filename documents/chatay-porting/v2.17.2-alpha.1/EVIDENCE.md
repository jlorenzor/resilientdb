# v2.17.2-alpha.1 Evidence

Date: 2026-06-30

## Build

```bash
docker run --rm \
  -v "${PWD}:/workspace" \
  -w /workspace \
  -v chatay-bazel-cache-v215:/root/.cache/bazel \
  chatay-resilientdb-toolchain:bazel6-20260528 \
  bash -lc "bazel build //benchmark/protocols/hs2:hs2_qc_boundary_probe //benchmark/protocols/hs2:hs2_new_txn_pipeline_probe //benchmark/protocols/hs2:kv_service --jobs=4"
```

Result:

```txt
Build completed successfully
```

## Probes

```bash
bazel run //benchmark/protocols/hs2:hs2_qc_boundary_probe --jobs=4
bazel run //benchmark/protocols/hs2:hs2_new_txn_pipeline_probe --jobs=4
```

Result:

```txt
hs2 qc boundary ok
hs2 new txn pipeline ok
```

## Smoke

```bash
CHATAY_SEMVER=v2.17.2-alpha.1 \
HS2_BASE_PORT=26200 \
HS2_OPERATION_COUNT=1 \
HS2_READY_TIMEOUT_SEC=90 \
HS2_CLIENT_TIMEOUT_SEC=45 \
BAZEL_JOBS=4 \
RUN_ID=20260630T-v2172-hs2-kv \
tools/chatay/hs2/run_hs2_kv_smoke_cluster.sh
```

Result:

```txt
all processes ready (5/5)
HS2 KV smoke-cluster passed operations=1/1
```

Local logs:

```txt
documents/chatay-porting/v2.17.2-alpha.1/logs/20260630T-v2172-hs2-kv/
```

The log directory is intentionally ignored because it contains generated keys,
certificates and process logs.
