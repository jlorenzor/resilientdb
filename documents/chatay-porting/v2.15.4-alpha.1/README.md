# v2.15.4-alpha.1 - HS2 NewView, highQC and Timeout Certificate

## Status

Closed.

## Implemented

- `Hs2Pacemaker` tracks current view and leader.
- `Hs2TimeoutCertificate` forms a timeout certificate when a quorum of timeout
  votes is present.
- `Hs2NewView` selects a highQC candidate and advances the leader/view path.

## Validation

```txt
HS2_TIMEOUT_CERTIFICATE_FORMED view=1 quorum=3 voters=2,3,4 high_qc_view=1 high_qc_block=block-1 next_view=2 next_leader=2
hs2 timeout certificate ok
HS2_NEW_VIEW_FORMED new_view=2 quorum=3 voters=2,3,4 selected_high_qc_view=1 selected_high_qc_block=block-1 leader=2
HS2_HIGH_QC_SELECTED view=1 block=block-1
hs2 new-view highqc ok
```

## Boundary

This validates pacemaker/new-view behavior in-process. It does not yet prove
liveness under an asynchronous network.
