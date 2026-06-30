# Segmented Benchmark Runner Plan - v2.18.2-alpha.1

Run ID: `20260630T235818Z-segmented-runner`

## Protocols

| Protocol | Runtime image |
| --- | --- |
| PBFT | `chatay-resilientdb-pbft:runtime-v2.13.20-alpha.1` |
| HS1/PR100 | `chatay-resilientdb-hs1-pr100:v2.14.10-beta.1` |
| HS2 | `chatay-resilientdb-hs2:v2.18.0-alpha.1` |

## Segments

| Segment | v2.18.2 behavior | Later gate |
| --- | --- | --- |
| `image-check` | Executed now with Docker inspect. | Input gate for all benchmark runs. |
| `build` | Planned and represented in the schema. | Real timing in a later build/runtime gate. |
| `cold-start` | Planned and represented in the schema. | Real timing in `v2.18.4-beta.1`. |
| `warm-cluster` | Planned and represented in the schema. | Real timing in `v2.18.3-beta.1`. |
| `phase-trace` | Planned and represented in the schema. | Real traces in `v2.18.5-rc.1`. |

## Claim Boundary

This alpha runner validates image availability and creates a common output schema. It does not yet produce comparative performance claims.
