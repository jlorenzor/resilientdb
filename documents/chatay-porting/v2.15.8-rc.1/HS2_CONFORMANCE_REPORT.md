# HS2 Conformance Report

Version: `v2.15.8-rc.1`

## Executive Summary

The fork now contains a C++/Bazel HS2 implementation track under
`platform/consensus/ordering/hs2`. It includes core protocol objects, quorum
certificate validation, timeout certificate handling, pacemaker/new-view logic,
negative safety probes and a runnable KV integration path.

The correct academic wording is **HS2 experimental fork implementation**. It is
not yet a complete production replacement for ResilientDB PBFT and should not be
presented as a finalized, formally verified HotStuff-2 implementation.

## Implemented Elements

| Element | Status | Path |
|---|---|---|
| C++/Bazel module | Implemented | `platform/consensus/ordering/hs2` |
| Block/vote/QC model | Implemented | `hs2_types.h`, `hs2_consensus.*` |
| Safety probes | Implemented | `hs2_safety_probe.cpp`, `hs2_negative_safety_probe.cpp` |
| Timeout certificate | Implemented | `hs2_timeout_certificate.*` |
| Pacemaker/view advance | Implemented | `hs2_pacemaker.*` |
| NewView/highQC selection | Implemented | `hs2_new_view.*` |
| ResilientDB manager adapter | Implemented | `consensus_manager_hs2.*` |
| KV service build | Implemented | `benchmark/protocols/hs2:kv_service` |
| 4-replica local smoke | Passed | `v2.15.6-beta.1` |

## Validation Summary

```txt
hs2 skeleton ok
hs2 safety ok
hs2 wiring ok
hs2 pacemaker ok
hs2 timeout certificate ok
hs2 new-view highqc ok
hs2 negative safety ok
HS2 KV smoke-cluster passed operations=1/1
```

## Current Limitations

| Limitation | Impact |
|---|---|
| Aggregate cryptographic signatures are represented by voter sets in probes | Adequate for state-machine validation, insufficient for production security claims. |
| `TYPE_NEW_TXNS` currently commits through `MessageManagerBasic` | The KV path is runnable, but not yet the final HS2 consensus message pipeline. |
| Fault validation is probe-level | More destructive multi-process tests are needed before comparative benchmarks. |
| No formal proof is embedded in the fork | Correctness arguments must reference the literature and explain implementation boundaries. |

## Accepted Claims

1. HS2 now exists as a C++/Bazel module inside the ResilientDB fork.
2. HS2 core safety, timeout and new-view components are executable and probed.
3. HS2 has a runnable KV service path.
4. A local 4-replica + 1-client-process smoke test completed a SET/GET flow.
5. HS2 can now be used as a controlled experimental implementation track.

## Rejected Claims

1. HS2 is already a complete production consensus replacement for PBFT.
2. HS2 has already been benchmarked fairly against PBFT and HS1.
3. HS2 fault tolerance has been fully demonstrated under real network partitions.

## Closing Decision

`v2.15.8-rc.1` closes HS2 as an experimental implementation candidate ready for
baseline preparation. It should enter `v2.16.0` only as a benchmark candidate
with explicit limitations.
