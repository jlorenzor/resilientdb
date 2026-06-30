# v2.18.1-alpha.1 - Protocol Image Inventory

## Status

Closed as the image-locking gate before segmented benchmark execution.

## Purpose

`v2.18.1-alpha.1` prevents benchmark drift by recording exactly which Docker
image represents each protocol family:

```txt
PBFT baseline
HS1/PR100 bridge
HS2 experimental runtime
```

The next runner must use these locked image identities instead of floating tags.

## Expected Inputs

```txt
chatay-resilientdb-pbft:runtime-v2.13.20-alpha.1
chatay-resilientdb-hs1-pr100:v2.14.10-beta.1
chatay-resilientdb-hs2:v2.18.0-alpha.1
```

## Generated Evidence

Run:

```bash
bash tools/chatay/images/inventory_protocol_images.sh
```

The script writes:

```txt
documents/chatay-porting/v2.18.1-alpha.1/protocol-image-inventory.json
documents/chatay-porting/v2.18.1-alpha.1/protocol-image-inventory.md
```

## Locked Images

| Protocol | Image | Approximate size |
| --- | --- | ---: |
| PBFT | `chatay-resilientdb-pbft:runtime-v2.13.20-alpha.1` | 1011 MB |
| HS1/PR100 | `chatay-resilientdb-hs1-pr100:v2.14.10-beta.1` | 976 MB |
| HS2 | `chatay-resilientdb-hs2:v2.18.0-alpha.1` | 976 MB |

The older build/toolchain images remain useful for rebuilds, but they are not
part of the comparable runtime benchmark set.

## Claim Boundary

This milestone only locks local image identities and sizes. It does not claim
that PBFT, HS1 and HS2 have already been benchmarked under equivalent runtime
conditions.

The benchmark comparison starts only after `v2.18.2-alpha.1` provides the
segmented runner.
