# v2.17.1-alpha.1 Checklist

## Goal

Close the immediate HS2 `TYPE_NEW_TXNS` hardening gap identified in
`v2.17.0-alpha.1`: the KV path must not commit without first crossing the HS2
block/QC/safety gate.

## Completed

- [x] Create branch `consensus/hs2-new-txns-pipeline-v2.17.1-alpha.1`.
- [x] Add `Hs2NewTxnPipeline`.
- [x] Build HS2 block from ResilientDB `Request`.
- [x] Build local phase-1 QC.
- [x] Build local phase-2 QC.
- [x] Attach `hs2-alpha-commit` proof to `Request.committed_certs`.
- [x] Route `ConsensusManagerHs2::HandleNewTransactions` through the pipeline.
- [x] Add probe target `//benchmark/protocols/hs2:hs2_new_txn_pipeline_probe`.
- [x] Validate first certification.
- [x] Validate duplicate/idempotent certification.
- [x] Validate next request extends the previous committed block.
- [x] Fix the initial `GET` timeout caused by unsafe synthetic parent hashes.
- [x] Validate 4-replica smoke cluster with one KV `SET` and one KV `GET`.
- [x] Keep heavy smoke logs ignored under `documents/chatay-porting/v2.17.1-alpha.1/logs`.

## Deferred

- [ ] Real networked HS2 votes.
- [ ] Cryptographic QC aggregation or signature verification in the HS2 runtime path.
- [ ] Full `TYPE_CUSTOM_CONSENSUS` HS2 message routing.
- [ ] Stopped-node, stopped-leader and slow-replica destructive scenarios.
- [ ] Warm-cluster repeated runs suitable for benchmark preconditions.
