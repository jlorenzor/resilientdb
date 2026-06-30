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

- [ ] Validate default-compatible run.
- [ ] Validate reduced-wait experimental run.
- [ ] Confirm `RUNTIME_SMOKE_PASSED`.
- [ ] Confirm policy traces appear in node logs.

## Exit Criteria

```txt
status=RUNTIME_SMOKE_PASSED
send_newview_policy traces present
defaults remain compatible
reduced-wait mode is runnable or explicitly classified
```
