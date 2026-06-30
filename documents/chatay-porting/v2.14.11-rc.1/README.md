# v2.14.11-rc.1 - HS1 Conformance Report

## Objective

Close the HS1/PR100 research gate with a defensible conformance report. This
version explains what the current fork implements, how it maps to the HotStuff
paper terminology, and where it remains an implementation-specific adaptation
inside ResilientDB.

## Deliverables

- `HS1_CONFORMANCE_REPORT.md`: technical conformance matrix.
- `CHECKLIST.md`: acceptance checklist for this release candidate.

## Boundary

This version does not claim that HS1/PR100 is a full, formally verified
implementation of canonical HotStuff. It claims that the fork contains a
HotStuff-like C++ ordering module wired to the ResilientDB KV path, with
observable phases, QC handling, quorum gating, signature checks, warm-cluster
evidence and a frozen runtime image.
