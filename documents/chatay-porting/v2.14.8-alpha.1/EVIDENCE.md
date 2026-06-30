# v2.14.8-alpha.1 Evidence - HS1 NEWVIEW Bootstrap Parameters

## Result

`v2.14.8-alpha.1` passed both the default-compatible run and the reduced-wait
experimental run.

## Default-Compatible Run

```txt
runId=20260630T043337Z-hs1-kv
status=RUNTIME_SMOKE_PASSED
branch=consensus/hs1-newview-bootstrap-v2.14.8-alpha.1
commit=e2f837865fd4264f407f05e9e9f939bea10d5509
operationCount=1
passedOperations=1
```

Node 1 effective policy:

```txt
newview_bootstrap_wait_start seconds=3 base_seconds=2 stagger_by_id=1
send_newview_policy attempts=3 retry_sleep_sec=3
readiness_reached replica_num=3 quorum=3
```

## Reduced-Wait Experimental Run

```txt
runId=20260630T043427Z-hs1-kv
status=RUNTIME_SMOKE_PASSED
branch=consensus/hs1-newview-bootstrap-v2.14.8-alpha.1
commit=e2f837865fd4264f407f05e9e9f939bea10d5509
operationCount=1
passedOperations=1
```

Environment:

```txt
CHATAY_HS1_NEWVIEW_BOOTSTRAP_BASE_SEC=0
CHATAY_HS1_NEWVIEW_BOOTSTRAP_STAGGER_BY_ID=0
CHATAY_HS1_NEWVIEW_ATTEMPTS=1
CHATAY_HS1_NEWVIEW_RETRY_SLEEP_SEC=0
```

Node 1 effective policy:

```txt
newview_bootstrap_wait_start seconds=0 base_seconds=0 stagger_by_id=0
send_newview_policy attempts=1 retry_sleep_sec=0
readiness_reached replica_num=3 quorum=3
```

## Interpretation Boundary

The reduced-wait run shows that the previous fixed sleeps are not always
required for a local 4-replica smoke run. This is not yet a statistically valid
cold-start optimization result. `v2.14.9-beta.1` must repeat this across
multiple runs and classify failures.
