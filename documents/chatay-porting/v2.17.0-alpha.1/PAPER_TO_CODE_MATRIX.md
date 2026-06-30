# HS2 Paper-to-Code Matrix

Version: `v2.17.0-alpha.1`

Branch: `consensus/hs2-paper-to-code-v2.17.0-alpha.1`

Base implementation branch: `consensus/hs2-fork-port-v2.15.0-alpha.1`

## Purpose

This matrix maps the current experimental HS2 implementation in the ResilientDB
fork against the algorithmic elements that must exist before Chatay can claim a
fair PBFT vs HS1 vs HS2 comparison.

The goal is not to overstate the implementation. The goal is to make the next
hardening steps unambiguous.

## Current Claim Boundary

Accepted wording:

```txt
The fork contains an experimental C++/Bazel HS2 path with core protocol
objects, safety probes, timeout certificate handling, NewView/highQC logic,
pacemaker support and a four-replica KV smoke test.
```

Rejected wording:

```txt
HS2 is already a production-grade replacement for ResilientDB PBFT.
HS2 has already been fairly benchmarked against PBFT and HS1.
HS2 has already demonstrated complete Byzantine fault tolerance under
multi-process network faults.
```

## Implementation Inventory

| Area | Path |
| --- | --- |
| HS2 core module | `platform/consensus/ordering/hs2` |
| HS2 service adapter | `platform/consensus/ordering/hs2/consensus_manager_hs2.*` |
| HS2 types | `platform/consensus/ordering/hs2/hs2_types.h` |
| HS2 core safety rules | `platform/consensus/ordering/hs2/hs2_consensus.*` |
| HS2 timeout certificates | `platform/consensus/ordering/hs2/hs2_timeout_certificate.*` |
| HS2 NewView/highQC | `platform/consensus/ordering/hs2/hs2_new_view.*` |
| HS2 pacemaker | `platform/consensus/ordering/hs2/hs2_pacemaker.*` |
| HS2 protobuf surface | `proto/hs2.proto` |
| HS2 probes | `benchmark/protocols/hs2/*_probe.cpp` |
| HS2 KV service | `benchmark/protocols/hs2/kv_service.cpp` |
| HS2 smoke runner | `tools/chatay/hs2/run_hs2_kv_smoke_cluster.sh` |

## Matrix

| Algorithmic element | Literature intent | Current code | Status | Gap | Next SemVer |
| --- | --- | --- | --- | --- | --- |
| Replica set and quorum | Operate with `n >= 3f + 1` and quorums of `2f + 1`. | `Hs2Consensus::QuorumSize`, `Hs2TimeoutCollector::QuorumSize`, `Hs2NewViewCollector::QuorumSize`. | Partial | Quorum is represented as voter ids; no cryptographic quorum aggregation yet. | `v2.17.2-alpha.1` |
| View | Each proposal and vote belongs to a logical view. | `Hs2Block.view`, `Hs2Vote.view`, `ConsensusManagerHs2::current_view_`, `Hs2Pacemaker::CurrentView`. | Partial | Runtime view transitions are only lightly wired; destructive leader failure is not validated. | `v2.17.4-alpha.1` |
| Leader per view | One leader/proposer drives a view. | `Hs2Pacemaker::LeaderForView`, `ConsensusManagerHs2::leader_id_`, `GetPrimary`, `SetPrimary`. | Partial | Leader election exists, but proposal ownership is not yet enforced through a complete HS2 message pipeline. | `v2.17.1-alpha.1` |
| Block/proposal | Leader proposes a block with parent, payload digest and justification. | `Hs2Block`, `Hs2Proposal` in `proto/hs2.proto`, `Hs2Consensus::CanVote`. | Partial | C++ core has block fields, but runtime proposal messages are not yet fully serialized and broadcast as HS2 consensus messages. | `v2.17.1-alpha.1` |
| Payload digest | Consensus votes over a digest, not arbitrary mutable application state. | `Hs2Block.payload_digest`, `proto.Hs2Block.payload_digest`. | Partial | Digest is modeled but not yet end-to-end bound to KV request execution in the HS2 commit path. | `v2.17.3-alpha.1` |
| Vote | Replicas vote for a block/phase/view. | `Hs2Vote`, `Hs2Consensus::RecordVote`, `proto.Hs2Vote`. | Partial | Duplicate/conflicting vote detection exists in probes; no production-grade signature verification in the HS2 path. | `v2.17.2-alpha.1` |
| Quorum Certificate | A QC proves quorum agreement for a block/phase/view. | `Hs2QuorumCertificate`, `Hs2Consensus::IsValidQuorumCertificate`, `proto.Hs2QuorumCertificate`. | Partial | QC currently validates voter ids and quorum size; aggregate signatures are represented by metadata/probes. | `v2.17.2-alpha.1` |
| Two-phase HS2 progress | HS2 should reduce consensus phases compared with chained HotStuff under the intended assumptions. | `Hs2Phase::kPhase1`, `Hs2Phase::kPhase2`, `WouldCommit` for `kPhase2`. | Partial | Phase labels exist, but the runtime message loop does not yet enforce an end-to-end two-phase pipeline. | `v2.17.1-alpha.1` |
| Locking rule | A replica should avoid voting for unsafe conflicting proposals. | `locked_qc_`, `SetLockedQc`, `IsSafeProposal`. | Partial | Locking is probed locally; it must be wired to the runtime state of each replica. | `v2.17.2-alpha.1` |
| Safe proposal rule | A proposal is safe if it extends the lock or carries a higher valid justification. | `Hs2Consensus::IsSafeProposal`, `Hs2NewViewCollector::IsSafeProposal`. | Partial | Rule exists as a function; runtime proposal path still needs to call it before accepting/committing client requests. | `v2.17.3-alpha.1` |
| Commit rule | A block commits only after sufficient certified progress. | `Hs2Consensus::WouldCommit`. | Incomplete | `TYPE_NEW_TXNS` currently reaches `MessageManagerBasic::Commit`, so the KV smoke does not yet prove HS2-native commit. | `v2.17.1-alpha.1` |
| Timeout message | Replica emits timeout with local highQC. | `Hs2TimeoutMessage`, `proto.Hs2Timeout`. | Partial | Timeout objects are validated in probes; runtime timeout emission is not yet part of the networked service. | `v2.17.4-alpha.1` |
| Timeout Certificate | A TC is formed from quorum timeout messages. | `Hs2TimeoutCollector`, `Hs2TimeoutCertificate`, `proto.Hs2TimeoutCertificate`. | Partial | TC validation exists; signatures and runtime propagation remain incomplete. | `v2.17.2-alpha.1` |
| NewView | New leader gathers view-change evidence and selects highQC. | `Hs2NewViewCollector`, `Hs2NewViewMessage`, `proto.Hs2Timeout`. | Partial | Collector and highQC selection exist; runtime NewView messages are not yet fully integrated into `ConsensusCommit`. | `v2.17.4-alpha.1` |
| highQC selection | New leader selects highest safe QC from NewView messages. | `Hs2NewViewCollector::SelectedHighQc`. | Partial | Highest-QC selection exists, but it needs networked validation with slow/stopped replicas. | `v2.17.4-alpha.1` |
| Pacemaker | Move to next view on timeout/failure. | `Hs2Pacemaker::AdvanceView`, `LeaderForView`. | Partial | Pacemaker can advance views, but it is not yet driven by real timeout certificates in the service loop. | `v2.17.4-alpha.1` |
| Client request ingestion | Client requests enter consensus through the leader/proxy path. | `ConsensusManagerHs2::HandleClientRequest`, `ResponseManager::NewUserRequestWithFallbackViews`. | Partial | Ingestion works with fallback views; HS2-native proposal creation is still missing. | `v2.17.1-alpha.1` |
| Transaction execution | Application execution happens only after consensus commit. | `benchmark/protocols/hs2/kv_service.cpp`, `KVExecutor`, `MemoryDB`. | Partial | KV service runs, but commit still relies on common basic manager, not full HS2 certification. | `v2.17.3-alpha.1` |
| Consensus message routing | HS2 messages should be routed through a protocol-specific message type. | `Request::TYPE_CUSTOM_CONSENSUS`, `ConsensusManagerHs2::HandleConsensusMessage`. | Incomplete | `HandleConsensusMessage` returns `kNotImplementedYet`. | `v2.17.1-alpha.1` |
| Fault handling | Safety/liveness must be tested with stopped leader, stopped node and slow replica. | `hs2_negative_safety_probe.cpp`, `Hs2Pacemaker::AdvanceView`. | Partial | Probe-level only; no destructive multi-process harness yet. | `v2.17.4-alpha.1` |
| Repeatability | Runtime evidence must survive repeated runs. | `run_hs2_kv_smoke_cluster.sh`; v2.15.6 evidence is 1/1 SET/GET. | Incomplete | Need 30/30 and 100/100 HS2 runs before benchmark. | `v2.17.5-beta.1` |
| Conformance report | Differences from literature must be stated before benchmarking. | `documents/chatay-porting/v2.15.8-rc.1/HS2_CONFORMANCE_REPORT.md`. | Partial | Must be updated after hardening and fault runs. | `v2.17.6-rc.1` |

## Critical Gaps

### G1: `TYPE_NEW_TXNS` bypass

Current code:

```cpp
return message_manager_->Commit(std::move(request));
```

Path:

```txt
platform/consensus/ordering/hs2/consensus_manager_hs2.cpp
```

Impact:

The runtime KV smoke proves that the HS2 service path can start and process a
minimal operation, but it does not yet prove that every transaction is committed
through the HS2 two-phase certification logic.

Required fix:

`v2.17.1-alpha.1` must introduce HS2-native consensus message handling for
proposal, vote, QC and commit, or explicitly wrap `MessageManagerBasic` only as
transport while the HS2 core decides commit.

### G2: Cryptographic quorum is still modeled

Current implementation validates quorum with voter ids. This is useful for
state-machine probes but insufficient for production claims.

Required fix:

`v2.17.2-alpha.1` must either integrate existing ResilientDB signature
verification for HS2 messages or document a controlled experimental substitute
that is not used for final security claims.

### G3: Fault evidence is probe-level

The current negative safety probe rejects malformed QC/TC/NewView cases and
models leader advance, but it does not stop real replica processes.

Required fix:

`v2.17.4-alpha.1` must add a destructive multi-process harness with at least:

1. leader stopped before proposal;
2. leader stopped after proposal;
3. one slow replica;
4. one stopped non-leader replica;
5. restart after timeout.

### G4: Repeatability is too small

`v2.15.6-beta.1` proves 1/1 HS2 KV operation. That is a smoke test, not a
benchmark-ready run.

Required fix:

`v2.17.5-beta.1` must run 30/30 and 100/100 operations and export structured
evidence.

## v2.17.x Acceptance Gate

HS2 may enter the local comparative benchmark only when:

- `TYPE_NEW_TXNS` no longer bypasses HS2 commit rules;
- HS2 consensus messages are routed through a protocol-specific path;
- vote/QC/TC validation has either real signatures or a documented experimental
  substitute;
- multi-process fault scenarios execute with logs;
- repeatability reaches 30/30 and 100/100 operations;
- the conformance report explicitly says which claims are still excluded.

## Immediate Next Work

The next version should be:

```txt
v2.17.1-alpha.1 - HS2 TYPE_NEW_TXNS pipeline hardening
```

Main technical target:

```txt
Replace the current MessageManagerBasic commit shortcut with an HS2-native
proposal/vote/QC/commit path or a narrowly documented transport-only adapter.
```
