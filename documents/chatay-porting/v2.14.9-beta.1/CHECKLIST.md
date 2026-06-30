# v2.14.9-beta.1 Checklist

## Purpose

Repeat HS1 cold-start runs for default and reduced NEWVIEW policies.

## Implementation Tasks

- [x] Create branch `consensus/hs1-cold-start-validation-v2.14.9-beta.1`.
- [x] Add matrix runner for default vs reduced mode.
- [x] Keep raw logs ignored.
- [x] Emit summary CSV.

## Runtime Tasks

- [x] Run matrix with at least 3 repetitions per mode.
- [x] Confirm all runs reach `RUNTIME_SMOKE_PASSED`.
- [x] Record run IDs and summary path.
- [x] Keep claim boundary as repetition smoke, not benchmark.

## Result

```txt
defaultRuns=3
reducedRuns=3
passedRuns=6
failedRuns=0
summary=documents/chatay-porting/v2.14.9-beta.1/summary.csv
```
