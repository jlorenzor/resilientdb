# v2.15.5-alpha.1 - HS2 KV Path Integration

## Status

Closed.

## Implemented

- `ConsensusManagerHs2` implements the ResilientDB `ConsensusManager` shape.
- HS2 supports the request types needed by the KV path:
  - `TYPE_CLIENT_REQUEST`
  - `TYPE_RESPONSE`
  - `TYPE_NEW_TXNS`
  - `TYPE_CUSTOM_QUERY`
  - `TYPE_CUSTOM_CONSENSUS`
- `benchmark/protocols/hs2:kv_service` wires HS2 to `KVExecutor(MemoryDB)` and
  `ServiceNetwork`.
- `ResponseManager` can attach fallback views for leader recovery experiments.

## Validation

```txt
bazel build //benchmark/protocols/hs2:kv_service
Build completed successfully
```

## Boundary

The current KV path is a runnable integration path. The `TYPE_NEW_TXNS` handler
commits through `MessageManagerBasic`; therefore this stage is an experimental
HS2 integration baseline, not a full production-grade HS2 consensus replacement.
