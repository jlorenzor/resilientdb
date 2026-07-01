# Runtime Phase Trace Coverage - v2.18.5-rc.1

Source log root: `documents/chatay-porting/v2.18.4-beta.1/logs/20260701T021524Z-runtime-coldstart`

## Event Counts

| Protocol | Events extracted |
| --- | ---: |
| `pbft` | 753 |
| `hs1-pr100` | 1317 |
| `hs2` | 792 |

## Coverage Matrix

| Protocol | Category | Status | Count |
| --- | --- | --- | ---: |
| `pbft` | `config_load` | `OBSERVED` | 60 |
| `pbft` | `key_cert_load` | `OBSERVED` | 75 |
| `pbft` | `network_start` | `OBSERVED` | 45 |
| `pbft` | `readiness` | `OBSERVED` | 30 |
| `pbft` | `bootstrap_heartbeat_send` | `OBSERVED` | 108 |
| `pbft` | `bootstrap_heartbeat_receive` | `OBSERVED` | 405 |
| `pbft` | `pbft_commit` | `OBSERVED` | 24 |
| `pbft` | `client_set_ok` | `OBSERVED` | 3 |
| `pbft` | `client_get_ok` | `OBSERVED` | 3 |
| `hs1-pr100` | `config_load` | `OBSERVED` | 60 |
| `hs1-pr100` | `key_cert_load` | `OBSERVED` | 75 |
| `hs1-pr100` | `network_start` | `OBSERVED` | 45 |
| `hs1-pr100` | `readiness` | `OBSERVED` | 30 |
| `hs1-pr100` | `bootstrap_heartbeat_send` | `OBSERVED` | 152 |
| `hs1-pr100` | `bootstrap_heartbeat_receive` | `OBSERVED` | 656 |
| `hs1-pr100` | `hs1_consensus_commit` | `OBSERVED` | 221 |
| `hs1-pr100` | `hs1_prepare_vote` | `OBSERVED` | 24 |
| `hs1-pr100` | `hs1_precommit_vote` | `OBSERVED` | 24 |
| `hs1-pr100` | `hs1_commit_vote` | `OBSERVED` | 24 |
| `hs1-pr100` | `client_set_ok` | `OBSERVED` | 3 |
| `hs1-pr100` | `client_get_ok` | `OBSERVED` | 3 |
| `hs2` | `config_load` | `OBSERVED` | 60 |
| `hs2` | `key_cert_load` | `OBSERVED` | 75 |
| `hs2` | `network_start` | `OBSERVED` | 45 |
| `hs2` | `readiness` | `OBSERVED` | 30 |
| `hs2` | `bootstrap_heartbeat_send` | `OBSERVED` | 108 |
| `hs2` | `bootstrap_heartbeat_receive` | `OBSERVED` | 408 |
| `hs2` | `hs2_phase2_certified` | `OBSERVED` | 60 |
| `hs2` | `client_set_ok` | `OBSERVED` | 3 |
| `hs2` | `client_get_ok` | `OBSERVED` | 3 |
| `pbft` | `view_change_or_leader_failure` | `NOT_EXECUTED` | 0 |
| `hs1-pr100` | `view_change_or_leader_failure` | `NOT_EXECUTED` | 0 |
| `hs2` | `view_change_or_leader_failure` | `NOT_EXECUTED` | 0 |

## Claim Boundary

The extracted events are runtime log observations from a local no-fault run. They confirm observable startup, readiness, selected commit/certification markers and client-visible SET/GET success. They do not prove complete protocol conformance and do not include a real view-change or leader-failure scenario in this gate.
