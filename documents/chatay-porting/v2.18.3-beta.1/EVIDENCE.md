# v2.18.3-beta.1 Evidence

## Command

```bash
CHATAY_PROTOCOLS=pbft,hs1-pr100,hs2 \
CHATAY_OPERATION_COUNT=10 \
bash tools/chatay/benchmark/run_runtime_image_kv_cluster.sh
```

## Summary

| Protocol | Image | Status | Operations | Passed | Ready ms | Operations ms |
| --- | --- | --- | ---: | ---: | ---: | ---: |
| PBFT | `chatay-resilientdb-pbft:v2.18.3-alpha.1` | `PASSED` | 10 | 10 | 9 | 12040 |
| HS1/PR100 | `chatay-resilientdb-hs1-pr100:v2.14.10-beta.1` | `PASSED` | 10 | 10 | 13110 | 12722 |
| HS2 | `chatay-resilientdb-hs2:v2.18.0-alpha.1` | `PASSED` | 10 | 10 | 14 | 11621 |

## Observations

The HS1/PR100 local warm-cluster run still shows a larger readiness interval
than PBFT and HS2. This remains a point for the cold-start gate and phase-trace
gate, because readiness behavior can distort simple end-to-end comparisons.

The operation timing is broadly close across the three protocols in this small
local run. No thesis-level performance claim should be derived from this single
run.

## Stored Runtime Logs

The detailed logs are intentionally excluded from git under:

```txt
documents/chatay-porting/v2.18.3-beta.1/logs/20260701T020837Z-runtime-warm/
```

The logs include per-protocol manifests, node logs, client SET/GET logs,
readiness traces and operation CSV files.
