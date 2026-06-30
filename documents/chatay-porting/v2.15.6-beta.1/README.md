# v2.15.6-beta.1 - HS2 Smoke 4 Nodes

## Status

Closed.

## Runner

```txt
tools/chatay/hs2/run_hs2_kv_smoke_cluster.sh
```

## Post-Commit Evidence

```json
{
  "semver": "v2.15.6-beta.1",
  "runId": "20260630Tpostcommit-hs2-kv",
  "protocol": "hs2",
  "status": "RUNTIME_SMOKE_PASSED",
  "commit": "35909ce0900436ad41a49353920e3387b9958e71",
  "replicaCount": 4,
  "clientProcessCount": 1,
  "totalProcessCount": 5,
  "basePort": 25100,
  "operationCount": 1,
  "passedOperations": 1,
  "finishedAt": "2026-06-30T13:48:26Z"
}
```

## Client Evidence

```txt
client set ret = 0
client get value = chatay-hs2-value-20260630Tpostcommit-hs2-kv-001
```

## Boundary

This smoke test proves that the HS2 KV binary starts in a local 4-replica +
1-client-process topology and accepts a minimal SET/GET flow. It does not prove
Byzantine fault tolerance by itself.
