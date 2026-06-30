# v2.17.4-alpha.1 - HS2 Multi-Process Fault Harness

Branch: `consensus/hs2-hardening-v2.17.2-to-v2.17.6`

## Purpose

This version adds destructive multi-process scenarios around the HS2 KV smoke
runner. The goal is not to claim full Byzantine fault tolerance yet, but to
stop relying only on function-level probes.

## Implementation

Updated:

```txt
tools/chatay/hs2/run_hs2_kv_smoke_cluster.sh
```

New:

```txt
tools/chatay/hs2/run_hs2_fault_matrix.sh
```

The smoke runner now accepts:

| Variable | Meaning |
| --- | --- |
| `HS2_STOP_NODE_AFTER_READY` | Kills one process after all nodes report readiness. |
| `HS2_STOP_NODE_AFTER_READY_DELAY_SEC` | Delay before the kill. |
| `HS2_SLOW_NODE_ID` | Delays startup for a chosen node. |
| `HS2_SLOW_NODE_DELAY_SEC` | Slow-start delay. |

## Scenarios

| Scenario | Meaning | Result |
| --- | --- | --- |
| `baseline` | No injected fault. | Passed |
| `stopped_nonleader` | Stop node 3 after readiness. | Passed |
| `slow_start` | Delay node 4 startup. | Passed |
| `stopped_leader` | Stop node 1 after readiness. | Passed, classified |

`stopped_leader` is still marked as classified because the current run proves
the local KV smoke survives the injected process stop, but it does not yet prove
complete networked HS2 view-change semantics.

## Claim Boundary

Safe wording:

```txt
The HS2 fork now has a multi-process fault harness and completed local SET/GET
smoke scenarios for baseline, stopped non-leader, slow start, and stopped leader
in the current alpha runtime.
```

Unsafe wording:

```txt
HS2 is now fully Byzantine-fault tolerant under arbitrary leader equivocation,
network partitions, and adversarial scheduling.
```
