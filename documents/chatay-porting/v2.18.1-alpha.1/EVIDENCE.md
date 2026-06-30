# v2.18.1-alpha.1 Evidence

## Command

```bash
bash tools/chatay/images/inventory_protocol_images.sh
```

Observed result:

```txt
wrote documents/chatay-porting/v2.18.1-alpha.1/protocol-image-inventory.json
wrote documents/chatay-porting/v2.18.1-alpha.1/protocol-image-inventory.md
```

## Expected Result

The command must fail if any of the required protocol images is absent.

When all images exist, it must write:

```txt
documents/chatay-porting/v2.18.1-alpha.1/protocol-image-inventory.json
documents/chatay-porting/v2.18.1-alpha.1/protocol-image-inventory.md
```

## Interpretation

The inventory is the image gate for subsequent benchmarks. It separates:

```txt
image identity
image size
build lineage
runtime behavior
```

Runtime behavior remains outside this milestone.

## Locked Runtime Set

| Protocol | Runtime image | Image ID | Size |
| --- | --- | --- | ---: |
| PBFT | `chatay-resilientdb-pbft:runtime-v2.13.20-alpha.1` | `sha256:321310ca4691` | 1011 MB |
| HS1/PR100 | `chatay-resilientdb-hs1-pr100:v2.14.10-beta.1` | `sha256:685173b06c92` | 976 MB |
| HS2 | `chatay-resilientdb-hs2:v2.18.0-alpha.1` | `sha256:87d206e1a2ad` | 976 MB |

The heavy local toolchain/build images are excluded from the comparable runtime
set. They can be kept for rebuilds, but the benchmark runner must use the
runtime images above.
