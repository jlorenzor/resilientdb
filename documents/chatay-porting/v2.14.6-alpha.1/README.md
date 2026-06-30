# v2.14.6-alpha.1 - HS1 KV Runtime Reproduction

## Objective

Run HS1/PR100 from fork-built binaries in a local cluster and validate a KV
`set/get` path.

ResilientDB's KV runtime uses four consensus replicas plus a client/gateway
process. Therefore the runtime gate starts:

```txt
4 replica processes
1 client/gateway process
```

## Starting point

Branch:

```txt
consensus/hs1-kv-runtime-v2.14.6-alpha.1
```

Known passing build targets:

```txt
//platform/consensus/ordering/hotstuff:consensus
//benchmark/protocols/hotstuff:kv_service
//benchmark/protocols/hotstuff:kv_server_performance
```

## Runner

```txt
tools/chatay/hs1/run_hs1_kv_warm_cluster.sh
```

The runner performs:

1. Bazel build for the HS1 service and KV client tools.
2. Key/certificate generation for local processes.
3. ResilientDB config generation.
4. HS1 `kv_service` startup for four replicas and one client/gateway process.
5. Readiness detection from real ResilientDB logs.
6. KV `SET` and `GET`, each guarded by `HS1_CLIENT_TIMEOUT_SEC`.
7. Manifest and logs under `documents/chatay-porting/v2.14.6-alpha.1/logs`.

## Command

```bash
docker run --rm \
  -e HS1_CLIENT_TIMEOUT_SEC=30 \
  -e HS1_OPERATION_COUNT=30 \
  -v "<fork>:/workspace" \
  -v chatay-bazel-cache:/root/.cache/bazel \
  -w /workspace \
  chatay-resilientdb-toolchain:bazel6-20260528 \
  bash tools/chatay/hs1/run_hs1_kv_warm_cluster.sh
```

## Runtime checklist

- [x] Create branch `consensus/hs1-kv-runtime-v2.14.6-alpha.1`.
- [x] Locate existing config/key generation scripts.
- [x] Start script for 4 HS1 replicas plus 1 client/gateway process.
- [x] Capture one log file per process.
- [x] Add readiness timestamps.
- [x] Add timeout classification for client `SET` and `GET`.
- [x] Send KV `SET` step in runner.
- [x] Send KV `GET` step in runner.
- [x] Verify returned value in a successful run.
- [x] Save evidence under `documents/chatay-porting/v2.14.6-alpha.1/logs`.
- [x] Classify result as config/key/network/readiness/client/consensus.

## Validated run

```txt
runId=20260630T042133Z-hs1-kv
status=RUNTIME_SMOKE_PASSED
replicaCount=4
clientProcessCount=1
operationCount=30
passedOperations=30
ready=5/5
```

See `EVIDENCE.md` for the reproducible evidence summary. Raw runtime logs are
kept locally but are intentionally not committed because the generated folder
contains ephemeral private keys and certificates.

## Claim boundary

Do not benchmark HS1 yet. This block is only a functional runtime gate.
