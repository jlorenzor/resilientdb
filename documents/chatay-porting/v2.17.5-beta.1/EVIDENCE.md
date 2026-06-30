# v2.17.5-beta.1 Evidence

## Command

Executed from the ResilientDB fork root:

```bash
docker run --rm \
  -v "${PWD}:/workspace" \
  -w /workspace \
  -v chatay-bazel-cache-v215:/root/.cache/bazel \
  chatay-resilientdb-toolchain:bazel6-20260528 \
  bash -lc "chmod +x tools/chatay/hs2/run_hs2_repeatability_gate.sh tools/chatay/hs2/run_hs2_kv_smoke_cluster.sh && CHATAY_SEMVER=v2.17.5-beta.1 RUN_ID=20260630T-v2175-hs2-repeat HS2_REPEAT_BASE_PORT=26800 HS2_REPEAT_OPERATION_COUNTS=30,100 HS2_AFTER_SET_SLEEP_SEC=0.2 HS2_READY_TIMEOUT_SEC=90 HS2_CLIENT_TIMEOUT_SEC=45 BAZEL_JOBS=4 tools/chatay/hs2/run_hs2_repeatability_gate.sh"
```

## Summary

```csv
operationCount,runId,basePort,exitCode,status,errorCode,passedOperations,stdoutLog
30,20260630T-v2175-hs2-repeat-30,26800,0,RUNTIME_SMOKE_PASSED,,30,/workspace/documents/chatay-porting/v2.17.5-beta.1/logs/20260630T-v2175-hs2-repeat/30.stdout.log
100,20260630T-v2175-hs2-repeat-100,26900,0,RUNTIME_SMOKE_PASSED,,100,/workspace/documents/chatay-porting/v2.17.5-beta.1/logs/20260630T-v2175-hs2-repeat/100.stdout.log
```

## Interpretation

The repeatability gate confirms that the v2.17 HS2 runtime path can complete
larger local warm-cluster KV workloads without losing operation continuity.
This closes a practical gap between one-operation smoke runs and future
benchmark campaigns.

This evidence is not yet a latency, throughput or consensus-comparison result.
It is a pre-benchmark stability gate.
