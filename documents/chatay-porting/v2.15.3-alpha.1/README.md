# v2.15.3-alpha.1 - HS2 Safety Rules and Probes

## Status

Closed.

## Implemented

- Quorum validation rejects insufficient voter sets.
- Quorum validation rejects duplicate voters.
- Vote recording rejects conflicting votes from the same replica for the same
  height, view and phase.
- Locked QC checks prevent voting for unsafe proposals.

## Validation

```txt
hs2 safety ok
HS2_NEGATIVE_SAFETY_PASSED cases=13 duplicate_qc=reject insufficient_qc=reject conflicting_vote=reject insufficient_tc=reject duplicate_tc=reject invalid_new_view=reject unsafe_proposal=reject leader_advance=ok
```

## Boundary

These are deterministic safety probes. They are not a model checker and do not
replace a formal proof.
