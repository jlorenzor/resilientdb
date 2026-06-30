# v2.14.7-alpha.1 - HS1 Cold-Start Instrumentation

## Objective

Instrument HS1/PR100 cold-start inside the real ResilientDB fork so the next
analysis can separate runtime startup costs from steady-state consensus costs.

This version does not optimize cold-start yet. It only adds trace points.

## Instrumented Areas

- `benchmark/protocols/hotstuff/kv_service.cpp`
  - process entry;
  - config load start/finish;
  - consensus construction;
  - service network construction;
  - server run entry.
- `platform/config/resdb_config_utils.cpp`
  - config file read;
  - private key read;
  - certificate read;
  - safe key/certificate metadata logging.
- `platform/networkstrate/service_network.cpp`
  - acceptor creation;
  - async acceptor start;
  - service start;
  - input thread start;
  - acceptor run entry.
- `platform/networkstrate/consensus_manager.cpp`
  - consensus manager constructor;
  - signature verifier creation;
  - heartbeat thread start;
  - heartbeat send/receive;
  - readiness reached.
- `platform/consensus/ordering/hotstuff/consensus.cpp`
  - HotStuff start entry;
  - base consensus start finish;
  - commitment init start/finish.
- `platform/consensus/ordering/hotstuff/commitment.cpp`
  - bootstrap wait start/finish;
  - initial NEWVIEW send start;
  - NEWVIEW attempts.

## Trace Prefix

```txt
CHATAY_HS1_COLD_START
```

The previous fine-grained debug prefix remains available:

```txt
CHATAY_HS1_TRACE
```

## Security Note

This version removes the previous raw private-key `DebugString()` logging from
`GenerateResDBConfig`. Cold-start logs now record key/certificate metadata, not
private-key material.

## Validation Command

```bash
docker run --rm \
  -e CHATAY_SEMVER=v2.14.7-alpha.1 \
  -e HS1_CLIENT_TIMEOUT_SEC=30 \
  -e HS1_OPERATION_COUNT=1 \
  -v "<fork>:/workspace" \
  -v chatay-bazel-cache:/root/.cache/bazel \
  -w /workspace \
  chatay-resilientdb-toolchain:bazel6-20260528 \
  bash tools/chatay/hs1/run_hs1_kv_warm_cluster.sh
```

## Claim Boundary

This version proves that cold-start tracepoints exist and the HS1 KV smoke path
still works. It does not yet:

- reduce cold-start time;
- remove NEWVIEW bootstrap waits;
- benchmark PBFT vs HS1 vs HS2;
- validate behavior under faults.
