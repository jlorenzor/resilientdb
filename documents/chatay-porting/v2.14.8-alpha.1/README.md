# v2.14.8-alpha.1 - HS1 NEWVIEW Bootstrap Parameters

## Objective

Parameterize HS1/PR100 NEWVIEW bootstrap behavior so cold-start experiments can
separate protocol behavior from fixed local sleeps.

## Parameters

| Variable | Default | Meaning |
|---|---:|---|
| `CHATAY_HS1_NEWVIEW_BOOTSTRAP_BASE_SEC` | `2` | Base sleep before initial NEWVIEW. |
| `CHATAY_HS1_NEWVIEW_BOOTSTRAP_STAGGER_BY_ID` | `1` | Add `node_id` seconds to the base wait when enabled. |
| `CHATAY_HS1_NEWVIEW_ATTEMPTS` | `3` | Number of initial NEWVIEW send attempts. |
| `CHATAY_HS1_NEWVIEW_RETRY_SLEEP_SEC` | `3` | Sleep between NEWVIEW attempts. |

Defaults preserve the behavior validated in `v2.14.6-alpha.1` and
`v2.14.7-alpha.1`.

## Validation Commands

Default-compatible run:

```bash
docker run --rm \
  -e CHATAY_SEMVER=v2.14.8-alpha.1 \
  -e HS1_CLIENT_TIMEOUT_SEC=30 \
  -e HS1_OPERATION_COUNT=1 \
  -v "<fork>:/workspace" \
  -v chatay-bazel-cache:/root/.cache/bazel \
  -w /workspace \
  chatay-resilientdb-toolchain:bazel6-20260528 \
  bash tools/chatay/hs1/run_hs1_kv_warm_cluster.sh
```

Reduced-wait experimental run:

```bash
docker run --rm \
  -e CHATAY_SEMVER=v2.14.8-alpha.1 \
  -e HS1_CLIENT_TIMEOUT_SEC=30 \
  -e HS1_OPERATION_COUNT=1 \
  -e CHATAY_HS1_NEWVIEW_BOOTSTRAP_BASE_SEC=0 \
  -e CHATAY_HS1_NEWVIEW_BOOTSTRAP_STAGGER_BY_ID=0 \
  -e CHATAY_HS1_NEWVIEW_ATTEMPTS=1 \
  -e CHATAY_HS1_NEWVIEW_RETRY_SLEEP_SEC=0 \
  -v "<fork>:/workspace" \
  -v chatay-bazel-cache:/root/.cache/bazel \
  -w /workspace \
  chatay-resilientdb-toolchain:bazel6-20260528 \
  bash tools/chatay/hs1/run_hs1_kv_warm_cluster.sh
```

## Claim Boundary

This version gives experimental control over NEWVIEW bootstrap parameters. It is
not yet the multi-run cold-start validation gate; that belongs to
`v2.14.9-beta.1`.
