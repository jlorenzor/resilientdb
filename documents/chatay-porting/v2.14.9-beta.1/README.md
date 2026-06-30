# v2.14.9-beta.1 - HS1 Cold-Start Repetition Matrix

## Objective

Validate HS1/PR100 cold-start behavior across repeated runs after the NEWVIEW
bootstrap parameters introduced in `v2.14.8-alpha.1`.

## Runner

```txt
tools/chatay/hs1/run_hs1_newview_coldstart_matrix.sh
```

The matrix runner executes:

- `default` mode: default NEWVIEW bootstrap policy;
- `reduced` mode: no bootstrap wait, no stagger, one NEWVIEW attempt.

## Command

```bash
docker run --rm \
  -e CHATAY_SEMVER=v2.14.9-beta.1 \
  -e HS1_MATRIX_REPEAT_COUNT=3 \
  -e HS1_CLIENT_TIMEOUT_SEC=30 \
  -v "<fork>:/workspace" \
  -v chatay-bazel-cache:/root/.cache/bazel \
  -w /workspace \
  chatay-resilientdb-toolchain:bazel6-20260528 \
  bash tools/chatay/hs1/run_hs1_newview_coldstart_matrix.sh
```

## Output

```txt
documents/chatay-porting/v2.14.9-beta.1/logs/summary.csv
```

Raw per-run logs remain ignored because they contain generated key material.

## Claim Boundary

This version checks repetition and basic stability. It is still not the final
PBFT vs HS1 vs HS2 benchmark.
