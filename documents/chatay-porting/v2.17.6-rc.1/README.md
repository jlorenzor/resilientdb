# v2.17.6-rc.1 - HS2 Runtime Hardening Conformance

## Objective

This release candidate closes the v2.17 HS2 hardening sequence by consolidating
what was implemented, validated and still bounded after `v2.17.2-alpha.1`
through `v2.17.5-beta.1`.

It is a documentation and traceability release candidate. It does not add a new
runtime feature. Its purpose is to keep the implementation, roadmap and thesis
claims aligned before the benchmark-preparation line starts in `v2.18.0`.

## Included Artifacts

```txt
HS2_RUNTIME_HARDENING_CONFORMANCE_REPORT.md
CHECKLIST.md
EVIDENCE.md
```

## Summary

The fork now contains an experimental C++/Bazel HS2 path integrated with the
ResilientDB KV runtime. The path gates `TYPE_NEW_TXNS` before KV commit,
separates QC construction boundaries, binds request payloads to commit proofs,
classifies multi-process fault scenarios and passes repeated warm-cluster KV
workloads locally.

This is strong enough to move from skeleton/probe work into benchmark
preparation. It is not yet enough to claim production readiness or performance
superiority over PBFT or HS1.

## Next Gate

```txt
v2.18.0 - Benchmark pre-release candidate for PBFT vs HS1 vs HS2
```

The next line must freeze reproducible protocol images and separate:

```txt
build time
cold-start time
warm-cluster operation time
```
