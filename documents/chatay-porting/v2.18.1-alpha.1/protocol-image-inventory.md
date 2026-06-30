# Protocol Image Inventory - v2.18.1-alpha.1

This inventory locks the local Docker images used for the next comparable PBFT vs HS1 vs HS2 run.

| Protocol | Role | Image | Image ID | Size | Notes |
| --- | --- | --- | --- | ---: | --- |
| `pbft` | PBFT baseline | `chatay-resilientdb-pbft:runtime-v2.13.20-alpha.1` | `sha256:321310ca4691` | 1011 MB | ResilientDB PBFT runtime baseline |
| `hs1-pr100` | HS1/PR100 bridge | `chatay-resilientdb-hs1-pr100:v2.14.10-beta.1` | `sha256:685173b06c92` | 976 MB | HotStuff-like PR100 runtime bridge |
| `hs2` | HS2 experimental | `chatay-resilientdb-hs2:v2.18.0-alpha.1` | `sha256:87d206e1a2ad` | 976 MB | Post-hardening HS2 experimental runtime |

## Claim Boundary

This file proves local image identity and approximate size. It does not prove comparable runtime behavior by itself.
The segmented runner must still measure build time, cold-start time, warm-cluster operation time and phase traces separately.
