# v2.14.6-alpha.1 Checklist

## Purpose

Close the first runtime gate for HS1/PR100 inside the real ResilientDB fork.

## Implementation Tasks

- [x] Keep work on branch `consensus/hs1-kv-runtime-v2.14.6-alpha.1`.
- [x] Confirm HS1 service target:
  - `//benchmark/protocols/hotstuff:kv_service`
- [x] Confirm KV client target:
  - `//service/tools/kv/api_tools:kv_service_tools`
- [x] Confirm key/certificate tools:
  - `//tools:certificate_tools`
  - `//tools:key_generator_tools`
  - `//tools:generate_region_config`
- [x] Create Chatay fork runner:
  - `tools/chatay/hs1/run_hs1_kv_warm_cluster.sh`
- [x] Preserve generated evidence under:
  - `documents/chatay-porting/v2.14.6-alpha.1/logs/<run-id>`

## Runtime Tasks

- [x] Build required Bazel targets inside Docker.
- [x] Generate local keys/certificates.
- [x] Generate server and client configs.
- [x] Start 4 replica processes.
- [x] Start 1 client/gateway process.
- [x] Confirm readiness for all processes using:
  - `============ Server <id> is ready`
- [x] Execute KV `set`.
- [x] Execute KV `get`.
- [x] Confirm returned value equals written value.
- [x] Store `manifest.json`.
- [x] Classify result:
  - `RUNTIME_SMOKE_PASSED`
  - `READINESS_TIMEOUT`
  - `CLIENT_SET_FAILED`
  - `CLIENT_GET_FAILED`
  - `CLIENT_GET_VALUE_MISMATCH`
  - another explicit error code

## Exit Criteria

`v2.14.6-alpha.1` is not closed until a real run from this fork produces
evidence with:

```txt
status=RUNTIME_SMOKE_PASSED
replicaCount=4
clientProcessCount=1
KV SET ret=0
KV GET value == KV SET value
```

Validated run:

```txt
runId=20260630T042133Z-hs1-kv
status=RUNTIME_SMOKE_PASSED
operationCount=30
passedOperations=30
```

## Claim Boundary

This version proves a functional HS1/PR100 KV gate only. It does not prove:

- benchmark superiority;
- canonical HotStuff conformance;
- Byzantine fault tolerance under adversarial scheduling;
- cold-start optimization.
