# v2.17.4-alpha.1 Evidence

Date: 2026-06-30

## Command

```bash
CHATAY_SEMVER=v2.17.4-alpha.1 \
RUN_ID=20260630T-v2174-hs2-faults \
HS2_FAULT_BASE_PORT=26400 \
HS2_OPERATION_COUNT=1 \
HS2_READY_TIMEOUT_SEC=90 \
HS2_CLIENT_TIMEOUT_SEC=45 \
BAZEL_JOBS=4 \
tools/chatay/hs2/run_hs2_fault_matrix.sh
```

## Summary

```csv
scenario,runId,basePort,exitCode,status,errorCode,expectation
baseline,20260630T-v2174-hs2-faults-baseline,26400,0,RUNTIME_SMOKE_PASSED,,required-pass
stopped_nonleader,20260630T-v2174-hs2-faults-stopped_nonleader,26500,0,RUNTIME_SMOKE_PASSED,,required-pass
slow_start,20260630T-v2174-hs2-faults-slow_start,26600,0,RUNTIME_SMOKE_PASSED,,required-pass
stopped_leader,20260630T-v2174-hs2-faults-stopped_leader,26700,0,RUNTIME_SMOKE_PASSED,,classified
```

Local summary:

```txt
documents/chatay-porting/v2.17.4-alpha.1/logs/20260630T-v2174-hs2-faults/summary.csv
```

## Interpretation

The local alpha runtime survives the injected process-stop and slow-start
scenarios for a one-operation KV smoke. This is stronger than probe-only
evidence, but it remains below a full Byzantine fault campaign.
