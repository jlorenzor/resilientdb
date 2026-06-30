# v2.17.6-rc.1 Evidence

## Traceability

| SemVer | Commit | Evidence |
| --- | --- | --- |
| `v2.17.2-alpha.1` | `28628e3d` | Build, QC boundary probe, pipeline probe and 1/1 KV smoke. |
| `v2.17.3-alpha.1` | `c42baa0b` | Build, payload binding probe, tamper rejection path and 1/1 KV smoke. |
| `v2.17.4-alpha.1` | `eead71dc` | Fault matrix for baseline, stopped non-leader, slow start and stopped leader. |
| `v2.17.5-beta.1` | `283f9df6` | Repeatability gate with 30/30 and 100/100 warm-cluster operations. |

## Local Validation Summary

```txt
v2.17.2: hs2 qc boundary ok; hs2 new txn pipeline ok; smoke 1/1
v2.17.3: hs2 payload binding ok; hs2 new txn pipeline ok; smoke 1/1
v2.17.4: fault matrix scenarios passed/classified
v2.17.5: repeatability gate passed 30/30 and 100/100
```

## Evidence Locations

```txt
documents/chatay-porting/v2.17.2-alpha.1/EVIDENCE.md
documents/chatay-porting/v2.17.3-alpha.1/EVIDENCE.md
documents/chatay-porting/v2.17.4-alpha.1/EVIDENCE.md
documents/chatay-porting/v2.17.5-beta.1/EVIDENCE.md
```

Generated process logs remain local and ignored by git.
