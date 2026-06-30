# v2.14.10-beta.1 Checklist

## Purpose

Freeze `chatay-resilientdb-hs1-pr100:v2.14.10-beta.1`.

## Tasks

- [x] Create branch `consensus/hs1-runtime-image-v2.14.10-beta.1`.
- [x] Compile current HS1 binaries from the fork.
- [x] Create runtime Dockerfile.
- [x] Build image.
- [x] Inspect image size and labels.
- [x] Record evidence.

## Exit Criteria

```txt
image=chatay-resilientdb-hs1-pr100:v2.14.10-beta.1
sizeBytes <= 1250000000
source_commit label present
```

## Result

```txt
status=closed
imageId=sha256:685173b06c9274d1e6a63327143001a030915853d10daa5fbaf843b3bc2a5717
sizeBytes=976420827
sizeApprox=976MB
```
