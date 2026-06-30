# v2.14.8-alpha.1 Checklist

## Purpose

Make HS1/PR100 NEWVIEW bootstrap tunable for cold-start analysis.

## Implementation Tasks

- [x] Create branch `consensus/hs1-newview-bootstrap-v2.14.8-alpha.1`.
- [x] Preserve default behavior from `v2.14.7-alpha.1`.
- [x] Add `CHATAY_HS1_NEWVIEW_BOOTSTRAP_BASE_SEC`.
- [x] Add `CHATAY_HS1_NEWVIEW_BOOTSTRAP_STAGGER_BY_ID`.
- [x] Add `CHATAY_HS1_NEWVIEW_ATTEMPTS`.
- [x] Add `CHATAY_HS1_NEWVIEW_RETRY_SLEEP_SEC`.
- [x] Log invalid/clamped env values.
- [x] Log the effective NEWVIEW policy.

## Runtime Tasks

- [x] Validate default-compatible run.
- [x] Validate reduced-wait experimental run.
- [x] Confirm `RUNTIME_SMOKE_PASSED`.
- [x] Confirm policy traces appear in node logs.

## Exit Criteria

```txt
status=RUNTIME_SMOKE_PASSED
send_newview_policy traces present
defaults remain compatible
reduced-wait mode is runnable or explicitly classified
```

Validated runs:

```txt
defaultRunId=20260630T043337Z-hs1-kv
reducedWaitRunId=20260630T043427Z-hs1-kv
status=RUNTIME_SMOKE_PASSED
commit=e2f837865fd4264f407f05e9e9f939bea10d5509
```
