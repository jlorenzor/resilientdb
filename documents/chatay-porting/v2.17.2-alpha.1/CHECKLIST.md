# v2.17.2-alpha.1 Checklist

- [x] Add explicit QC boundary components.
- [x] Add vote signature material to `Hs2QuorumCertificate`.
- [x] Isolate alpha/local vote generation from QC construction.
- [x] Reject malformed/unsigned alpha votes.
- [x] Reject conflicting duplicate votes from the same replica.
- [x] Preserve idempotent duplicate handling for identical votes.
- [x] Add `hs2_qc_boundary_probe`.
- [x] Keep `hs2_new_txn_pipeline_probe` passing.
- [x] Keep 4-replica KV smoke passing.

## Deferred

- [ ] Replace alpha/local votes with real networked HS2 vote messages.
- [ ] Replace alpha signature strings with production cryptographic signatures
      or a formally documented threshold-signature substitute.
- [ ] Persist and inspect QC material across restart.
