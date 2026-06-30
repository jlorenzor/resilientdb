# HS1 Baseline Port Checklist - v2.14.4-alpha.1

## Completed

- [x] Created fork branch `consensus/hs1-baseline-tree-v2.14.4-alpha.1`.
- [x] Fetched Apache PR100 as `upstream/pr/100`.
- [x] Imported `platform/consensus/ordering/hotstuff` from PR100.
- [x] Imported PR100 root common dependencies needed by HotStuff.
- [x] Preserved current upstream `common/transaction_utils.*`.
- [x] Applied preserved benchmark/KV HotStuff patches from `v2.14.2-alpha.1`.
- [x] Added root common BUILD targets for PR100 helper classes.
- [x] Ran static BUILD label existence check.
- [x] Cleaned inherited trailing whitespace in `hotstuff.proto`.

## Deferred

- [ ] Run real `bazel query` in controlled toolchain.
- [ ] Build `//platform/consensus/ordering/hotstuff:consensus`.
- [ ] Build `//benchmark/protocols/hotstuff:kv_service`.
- [ ] Reconcile API drift discovered by Bazel.
- [ ] Produce HS1 runtime image.
- [ ] Reproduce HS1 warm-cluster KV flow.
- [ ] Instrument HS1 cold-start/readiness.
