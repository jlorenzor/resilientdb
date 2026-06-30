# v2.15.2-alpha.1 - HS2 Core State Objects

## Status

Closed.

## Implemented

- Block model with height, view, block hash, parent hash, payload digest and
  proposer id.
- Vote model with replica id, height, view, block hash and phase.
- Quorum certificate model with phase, voters and block metadata.
- Timeout certificate model through `Hs2TimeoutCertificate`.
- Locked/high QC state through `Hs2Consensus` and new-view helpers.

## Validation

Covered by:

```txt
hs2_skeleton_probe
hs2_safety_probe
hs2_timeout_certificate_probe
hs2_new_view_highqc_probe
```

## Boundary

The core state objects are local C++ objects. Cryptographic aggregate signatures
are represented by voter sets in this alpha/beta line and must not be described
as production threshold signatures.
