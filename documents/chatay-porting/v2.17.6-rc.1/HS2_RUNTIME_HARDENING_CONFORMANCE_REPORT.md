# HS2 Runtime Hardening Conformance Report

Version: `v2.17.6-rc.1`  
Date: 2026-06-30  
Repository: Chatay ResilientDB fork  
Branch: `consensus/hs2-hardening-v2.17.2-to-v2.17.6`

## 1. Executive Summary

The v2.17 line moves the HS2 work from a skeleton and probe-oriented port
toward an executable experimental runtime path inside the ResilientDB fork.
The implementation remains C++/Bazel-native and is exercised through the
ResilientDB KV service path.

The main result is that `TYPE_NEW_TXNS` can be gated by an experimental HS2
pipeline before KV commit, while the runtime now includes explicit QC
boundaries, payload-to-proof binding, basic fault scenario classification and
repeatable warm-cluster KV workloads.

This version is suitable for benchmark preparation. It is not yet a production
consensus replacement.

## 2. Versioned Progress

| Version | Commit | Contribution |
| --- | --- | --- |
| `v2.17.1-alpha.1` | `bf332927` | Gated `TYPE_NEW_TXNS` through the HS2 block/QC/safety pipeline before commit. |
| `v2.17.2-alpha.1` | `28628e3d` | Split vote collection, QC construction and verifier boundaries. |
| `v2.17.3-alpha.1` | `c42baa0b` | Bound payload digest, request hash and execution evidence into the commit proof. |
| `v2.17.4-alpha.1` | `eead71dc` | Added a multi-process fault matrix for baseline, stopped replica, slow startup and stopped leader scenarios. |
| `v2.17.5-beta.1` | `283f9df6` | Added a repeatability gate and passed 30/30 plus 100/100 warm-cluster operations. |
| `v2.17.6-rc.1` | current | Consolidates conformance, claim boundary and next gates. |

## 3. Implementation Inventory

| Area | Files | Role |
| --- | --- | --- |
| HS2 runtime pipeline | `platform/consensus/ordering/hs2/hs2_new_txn_pipeline.*` | Builds the HS2 proposal chain, applies safety checks and produces commit proof metadata before KV commit. |
| QC boundary | `platform/consensus/ordering/hs2/hs2_qc_boundary.*` | Separates vote recording, quorum construction and verification interfaces. |
| Payload binding | `platform/consensus/ordering/hs2/hs2_payload_binding.*` | Binds the request payload, request hash, block digest, phase proofs and execution digest. |
| Probes | `benchmark/protocols/hs2/*_probe.cpp` | Validate local safety, QC boundary, payload binding and wiring expectations. |
| Runtime smoke harness | `tools/chatay/hs2/run_hs2_kv_smoke_cluster.sh` | Starts local HS2 replica/client processes and validates SET/GET operation continuity. |
| Fault matrix | `tools/chatay/hs2/run_hs2_fault_matrix.sh` | Runs baseline, stopped non-leader, slow-start and stopped-leader scenarios. |
| Repeatability gate | `tools/chatay/hs2/run_hs2_repeatability_gate.sh` | Executes repeated warm-cluster workloads with configurable operation counts. |

## 4. Conformance Matrix

| Requirement | Current Status | Evidence | Boundary |
| --- | --- | --- | --- |
| Native ResilientDB implementation stack | Satisfied | C++/Bazel files under `platform/consensus/ordering/hs2` and `benchmark/protocols/hs2`. | The implementation is fork-local and not upstreamed. |
| KV path integration | Satisfied experimentally | `TYPE_NEW_TXNS` gated before KV commit; smoke tests pass. | Final production integration and operator controls remain pending. |
| HS2 block/QC/safety pipeline | Partially satisfied | Probes and KV smoke validate local block extension and safety path. | QC aggregation is still simplified and local. |
| QC boundary separation | Satisfied as an engineering boundary | `v2.17.2-alpha.1` build/probes/smoke. | Real cryptographic quorum aggregation remains pending. |
| Payload-to-execution binding | Satisfied for local proof metadata | `v2.17.3-alpha.1` payload binding probe and tamper rejection path. | Proof format is experimental and not a wire-compatible standard. |
| Fault scenario classification | Partially satisfied | `v2.17.4-alpha.1` fault matrix. | Does not yet model arbitrary Byzantine equivocation or network partitions. |
| Repeatable warm-cluster behavior | Satisfied locally | `v2.17.5-beta.1` passed 30/30 and 100/100. | Not a throughput or latency benchmark. |
| Comparative benchmark readiness | Not yet satisfied | Roadmap moves to `v2.18.0`. | Requires frozen images and segmented timing. |

## 5. Validation Evidence

The following validations were completed locally through Docker/Bazel:

```txt
v2.17.2-alpha.1
- Bazel build completed successfully
- hs2 qc boundary ok
- hs2 new txn pipeline ok
- HS2 KV smoke-cluster passed operations=1/1

v2.17.3-alpha.1
- Bazel build completed successfully
- hs2 payload binding ok
- hs2 new txn pipeline ok
- HS2 KV smoke-cluster passed operations=1/1

v2.17.4-alpha.1
- baseline: RUNTIME_SMOKE_PASSED
- stopped_nonleader: RUNTIME_SMOKE_PASSED
- slow_start: RUNTIME_SMOKE_PASSED
- stopped_leader: RUNTIME_SMOKE_PASSED, classified

v2.17.5-beta.1
- 30/30 operations passed
- 100/100 operations passed
```

## 6. Claim Boundary

The defensible claim after this milestone is:

```txt
The ResilientDB fork contains an experimental HS2 C++/Bazel runtime path
integrated with the KV service. The path gates new transactions before commit,
uses explicit local QC construction boundaries, binds request payloads to commit
proof metadata and has passed local smoke, fault-matrix and repeatability gates.
```

The following claims are not yet defensible:

```txt
HS2 is production-ready in this fork.
HS2 already outperforms PBFT or HS1.
The current QC path implements final cryptographic quorum aggregation.
The current fault campaign covers all Byzantine behaviors.
The current local warm-cluster tests are equivalent to a heterogeneous network benchmark.
```

## 7. Remaining Gaps

| Gap | Risk | Planned Direction |
| --- | --- | --- |
| Cryptographic QC aggregation is simplified. | The implementation can be challenged as an engineering approximation. | Add a dedicated cryptographic quorum certificate milestone before final claims. |
| Vote exchange is still local/simplified. | It may not represent the full protocol message cost. | Introduce explicit message-path instrumentation and, if needed, networked vote exchange. |
| Fault matrix is process-level. | It does not model all Byzantine behaviors. | Add scenarios for leader crash before proposal, replica delay, duplicate proposal and network partition. |
| Benchmarks are not frozen. | Results could be skewed by build and cold-start costs. | Freeze images and segment build, cold-start and warm-cluster timing. |
| Heterogeneous deployment is not yet validated. | Local Docker evidence is not enough for the thesis title. | Prepare ARM64/AMD64 images and a node profile matrix before distributed tests. |

## 8. Benchmark Preparation Requirements

Before comparing PBFT, HS1 and HS2, the benchmark line must provide:

```txt
protocol-specific Docker images
fixed build provenance
warm-cluster readiness checks
cold-start timing
operation-only timing
fault-scenario timing
CSV/JSON export
repeated-run summary
```

This is the target for `v2.18.0` and its follow-up release candidates.

## 9. Academic Wording

Recommended wording for the report:

```txt
En la linea v2.17 se materializo una ruta experimental de HotStuff-2 dentro del
fork de ResilientDB, integrada al flujo KV y validada mediante pruebas locales
de humo, escenarios de falla de proceso y ejecuciones repetidas. Esta evidencia
habilita la fase de preparacion de benchmarks comparativos, aunque aun no
constituye una afirmacion de superioridad de rendimiento ni una version de
produccion del protocolo.
```

Avoid wording that implies final superiority or full protocol equivalence before
the benchmark and conformance gates are closed.
