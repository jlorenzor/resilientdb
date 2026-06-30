# v2.14.7-alpha.1 Checklist

## Purpose

Instrument HS1/PR100 cold-start in the real ResilientDB fork.

## Implementation Tasks

- [x] Create branch `consensus/hs1-cold-start-v2.14.7-alpha.1`.
- [x] Add trace prefix `CHATAY_HS1_COLD_START`.
- [x] Instrument config file loading.
- [x] Instrument private key loading without logging private key material.
- [x] Instrument certificate loading.
- [x] Instrument consensus manager construction and heartbeat startup.
- [x] Instrument service network construction and acceptor startup.
- [x] Instrument HotStuff consensus start.
- [x] Instrument commitment init and initial NEWVIEW send.
- [x] Ensure versioned raw logs are ignored by Git.

## Runtime Tasks

- [x] Build required Bazel targets inside Docker.
- [x] Run HS1 KV smoke path with `HS1_OPERATION_COUNT=1`.
- [x] Confirm `RUNTIME_SMOKE_PASSED`.
- [x] Confirm cold-start traces appear in node logs.
- [x] Store evidence summary without committing raw keys/certificates.

## Exit Criteria

`v2.14.7-alpha.1` is closed when a run from the committed branch produces:

```txt
status=RUNTIME_SMOKE_PASSED
operationCount=1
passedOperations=1
CHATAY_HS1_COLD_START traces present
```

Validated run:

```txt
runId=20260630T043005Z-hs1-kv
status=RUNTIME_SMOKE_PASSED
branch=consensus/hs1-cold-start-v2.14.7-alpha.1
commit=0ff1429367593570f7d5a893575e8aa7738a376c
operationCount=1
passedOperations=1
```

## Claim Boundary

This version is observability-only. Cold-start corrections belong to
`v2.14.8-alpha.1` and later.
