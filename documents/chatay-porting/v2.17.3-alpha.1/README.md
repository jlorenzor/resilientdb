# v2.17.3-alpha.1 - HS2 Payload and Execution Binding

Branch: `consensus/hs2-hardening-v2.17.2-to-v2.17.6`

## Purpose

This version binds the `TYPE_NEW_TXNS` request payload to the HS2 commit proof.
The goal is to avoid a weak claim where HS2 certifies an abstract block but the
application payload can drift silently before KV execution.

## Implementation

New component:

```txt
platform/consensus/ordering/hs2/hs2_payload_binding.*
```

The alpha commit proof now carries:

| Field | Meaning |
| --- | --- |
| `payload_digest` | Digest derived from request hash/data. |
| `request_hash` | Digest over request type, sequence, view, sender/proxy and payload material. |
| `block_digest` | Digest of the HS2 block hash material. |
| `phase1_proof` | Digest of the phase-1 QC proof material. |
| `phase2_proof` | Digest of the phase-2 QC proof material. |
| `execution_digest` | Digest binding block, request and phase-2 proof before execution. |
| `binding_digest` | Final digest over the whole binding. |

## Validation

The new probe verifies that:

- a valid proof verifies against the original request;
- a tampered payload is rejected;
- proof fields required for methodology are present.

## Claim Boundary

This is not yet a full execution trace emitted by the KV executor. It is a
pre-execution binding proof that connects request payload, HS2 certification and
the expected execution boundary.

## Next

`v2.17.4-alpha.1` must add multi-process fault harnesses for stopped leader,
stopped replica and slow-start scenarios.
