# v2.17.5-beta.1 Checklist

| Item | Status | Evidence |
| --- | --- | --- |
| Add a repeatability runner for HS2 KV warm-cluster workloads. | Done | `tools/chatay/hs2/run_hs2_repeatability_gate.sh` |
| Preserve the existing four-replica smoke-cluster path. | Done | Runner delegates to `run_hs2_kv_smoke_cluster.sh`. |
| Execute 30 operations in the local Docker toolchain. | Done | `30/30` passed. |
| Execute 100 operations in the local Docker toolchain. | Done | `100/100` passed. |
| Emit a CSV summary for later thesis traceability. | Done | `summary.csv` under the local v2.17.5 logs directory. |
| Keep raw logs out of git. | Done | `logs/.gitignore`. |
| Avoid benchmark superiority claims. | Done | README and evidence mark this as repeatability only. |
