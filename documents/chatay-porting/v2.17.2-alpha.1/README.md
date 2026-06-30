# v2.17.2-alpha.1 - HS2 QC Boundary Hardening

Branch: `consensus/hs2-hardening-v2.17.2-to-v2.17.6`

## Purpose

This version reduces the main shortcut left by `v2.17.1-alpha.1`: quorum
material was still created directly inside the `TYPE_NEW_TXNS` pipeline. The
implementation now separates four explicit responsibilities:

| Component | Responsibility |
| --- | --- |
| `Hs2LocalVoteSource` | Produces alpha/local votes for the current experimental runtime. |
| `Hs2VoteVerifier` | Validates vote shape, slot, phase, replica id and alpha signature boundary. |
| `Hs2VoteCollector` | Collects votes by replica and rejects conflicting duplicate votes. |
| `Hs2QcBuilder` | Builds QC objects with voters, vote signatures and a proof digest. |

## Claim Boundary

This is still not full networked cryptographic QC exchange. The improvement is
architectural and testable: the synthetic/local vote source is now isolated
behind a boundary that can later be replaced by real network messages and real
signature verification.

## Changed Files

```txt
platform/consensus/ordering/hs2/hs2_qc_boundary.*
platform/consensus/ordering/hs2/hs2_new_txn_pipeline.*
platform/consensus/ordering/hs2/hs2_types.h
benchmark/protocols/hs2/hs2_qc_boundary_probe.cpp
```

## Outcome

`TYPE_NEW_TXNS` still passes through the HS2 pipeline, but phase QCs are now
created through a visible vote/QC boundary and the commit proof carries QC proof
digests.

## Next

`v2.17.3-alpha.1` must bind payload digest, request hash, commit proof and KV
execution evidence end-to-end.
