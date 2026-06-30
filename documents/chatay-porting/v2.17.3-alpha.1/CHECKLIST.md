# v2.17.3-alpha.1 Checklist

- [x] Add `Hs2CommitProofBuilder`.
- [x] Add payload digest binding.
- [x] Add request hash binding.
- [x] Add execution digest boundary.
- [x] Add final binding digest.
- [x] Attach enriched proof through `Request.committed_certs`.
- [x] Add proof verification for the original request.
- [x] Add tampered-payload rejection probe.
- [x] Keep HS2 pipeline probe passing.
- [x] Keep 4-replica KV smoke passing.

## Deferred

- [ ] Emit post-execution KV trace records from the executor.
- [ ] Persist execution traces to a benchmark artifact.
- [ ] Compare execution-bound proof against real networked QC material.
