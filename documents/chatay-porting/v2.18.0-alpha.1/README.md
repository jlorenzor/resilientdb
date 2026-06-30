# v2.18.0-alpha.1 - HS2 Runtime Image Freeze

## Objective

This milestone freezes a clean HS2 runtime image for the ResilientDB fork after
the `v2.17.6-rc.1` hardening conformance report.

The goal is to prepare benchmark work without mixing source build time, Bazel
cache behavior or full toolchain weight into runtime measurements.

## Image

```txt
chatay-resilientdb-hs2:v2.18.0-alpha.1
```

Image summary:

```txt
imageId=sha256:87d206e1a2ad9bc0d7af662479fa45e77da5f0c82238b67249450a6576bd1224
sizeBytes=976114081
sizeApprox=976MB
baseImage=chatay-resilientdb-hs2:runtime-v2.13.20-alpha.1
```

The image remains inside the expected runtime-image range used by the Chatay
consensus-image policy.

## Added Packaging Artifacts

```txt
tools/chatay/images/Dockerfile.hs2-runtime
tools/chatay/images/build_hs2_runtime_image.sh
```

The script:

1. builds HS2 runtime binaries inside the existing Bazel toolchain image;
2. copies only runtime binaries into a versioned runtime context;
3. builds the final runtime image from the previous lightweight HS2 base;
4. writes `image-summary.json`;
5. writes `runtime-content.txt`.

## Runtime Binaries

```txt
kv_service
kv_service_tools
certificate_tools
key_generator_tools
generate_region_config
```

## Claim Boundary

This milestone proves packaging identity and runtime-content availability. It
does not yet prove that the final image can execute the whole four-replica HS2
cluster without mounting the source tree.

That next runtime validation belongs to:

```txt
v2.18.1-alpha.1 - PBFT, HS1 and HS2 image inventory
v2.18.2-alpha.1 - Segmented runner for build, cold-start and warm-cluster timing
```
