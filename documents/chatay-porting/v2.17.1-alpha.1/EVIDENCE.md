# v2.17.1-alpha.1 Evidence

Date: 2026-06-30

Host context: Windows 11 + Docker Desktop, using the existing
`chatay-resilientdb-toolchain:bazel6-20260528` image and
`chatay-bazel-cache-v215` cache volume.

## Build

Command:

```bash
docker run --rm \
  -v "${PWD}:/workspace" \
  -w /workspace \
  -v chatay-bazel-cache-v215:/root/.cache/bazel \
  chatay-resilientdb-toolchain:bazel6-20260528 \
  bash -lc "bazel build //benchmark/protocols/hs2:hs2_new_txn_pipeline_probe //benchmark/protocols/hs2:hs2_wiring_probe //benchmark/protocols/hs2:kv_service --jobs=4"
```

Result:

```txt
Build completed successfully
```

## Probe

Command:

```bash
docker run --rm \
  -v "${PWD}:/workspace" \
  -w /workspace \
  -v chatay-bazel-cache-v215:/root/.cache/bazel \
  chatay-resilientdb-toolchain:bazel6-20260528 \
  bash -lc "bazel run //benchmark/protocols/hs2:hs2_new_txn_pipeline_probe --jobs=4"
```

Result:

```txt
hs2 new txn pipeline ok
Build completed successfully
```

The probe validates:

- first request certification;
- duplicate sequence idempotence;
- next request extends the previous committed block;
- invalid leader rejection.

## Initial Smoke Failure

The first 4-replica KV smoke run built and started all processes, but failed on
`GET`:

```txt
ERROR: client GET operation 001 timed out after 45s
```

Observed cause:

```txt
CHATAY_HS2_PIPELINE rejected seq=2 ... reason=proposal failed safety rule
```

Root cause:

```txt
The pipeline kept a locked QC after the first commit, but the next request used
a synthetic parent hash that did not match the committed block hash. The HS2
safety rule correctly rejected the proposal.
```

Fix:

```txt
Track last_committed_block_hash, last_committed_height and last_committed_view
inside Hs2NewTxnPipeline. New blocks extend the last committed block. Duplicate
sequence numbers are treated as already certified so broadcast retries do not
stall the executor.
```

## Passing Smoke

Command:

```bash
docker run --rm \
  -v "${PWD}:/workspace" \
  -w /workspace \
  -v chatay-bazel-cache-v215:/root/.cache/bazel \
  chatay-resilientdb-toolchain:bazel6-20260528 \
  bash -lc "CHATAY_SEMVER=v2.17.1-alpha.1 HS2_BASE_PORT=26100 HS2_OPERATION_COUNT=1 HS2_READY_TIMEOUT_SEC=90 HS2_CLIENT_TIMEOUT_SEC=45 BAZEL_JOBS=4 RUN_ID=20260630T-v2171-hs2-kv-fixed tools/chatay/hs2/run_hs2_kv_smoke_cluster.sh"
```

Result:

```txt
all processes ready (5/5)
operation 1/1: KV SET timeout=45s
operation 1/1: KV GET timeout=45s
HS2 KV smoke-cluster passed operations=1/1
```

Local logs:

```txt
documents/chatay-porting/v2.17.1-alpha.1/logs/20260630T-v2171-hs2-kv-fixed/
```

Logs are intentionally not committed because they contain generated keys,
certificates and process output.
