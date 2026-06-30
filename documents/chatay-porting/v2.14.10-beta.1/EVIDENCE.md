# v2.14.10-beta.1 Evidence - HS1 Runtime Image Freeze

## Status

Closed.

## Image

```txt
name=chatay-resilientdb-hs1-pr100:v2.14.10-beta.1
imageId=sha256:685173b06c9274d1e6a63327143001a030915853d10daa5fbaf843b3bc2a5717
sizeBytes=976420827
sizeApprox=976MB
policy=accepted-below-1.25GB-review-threshold
```

## Labels

```json
{
  "org.chatay.base_image": "chatay-resilientdb-hs1-pr100:runtime-v2.13.20-alpha.1",
  "org.chatay.consensus": "hs1-pr100",
  "org.chatay.image_role": "runtime-release",
  "org.chatay.semver": "v2.14.10-beta.1",
  "org.chatay.source_branch": "consensus/hs1-runtime-image-v2.14.10-beta.1",
  "org.chatay.source_commit": "ed4c1d7e3b1ecf168cd3bfc56eac864ca61d2eeb",
  "org.chatay.source_image": "chatay-resilientdb-hs1-pr100:44ce20a-warm-continuity-rc6"
}
```

## Runtime Manifest

```json
{
  "schemaVersion": "chatay-consensus-runtime-image-v2",
  "protocol": "hs1-pr100",
  "semver": "v2.14.10-beta.1",
  "sourceCommit": "ed4c1d7e3b1ecf168cd3bfc56eac864ca61d2eeb",
  "sourceBranch": "consensus/hs1-runtime-image-v2.14.10-beta.1",
  "runtimeWorkdir": "/opt/resilientdb-hs1",
  "binaries": [
    "kv_service",
    "kv_service_tools",
    "certificate_tools",
    "key_generator_tools",
    "generate_region_config"
  ]
}
```

## Runtime Content Check

```txt
/opt/resilientdb-hs1/bin/certificate_tools      6.1M executable
/opt/resilientdb-hs1/bin/generate_region_config 21K executable
/opt/resilientdb-hs1/bin/key_generator_tools    5.0M executable
/opt/resilientdb-hs1/bin/kv_service             9.4M executable
/opt/resilientdb-hs1/bin/kv_service_tools       7.3M executable
```

## Command Evidence

```powershell
docker image inspect chatay-resilientdb-hs1-pr100:v2.14.10-beta.1 --format '{{.Id}} {{.Size}} {{json .Config.Labels}}'
docker run --rm --entrypoint sh chatay-resilientdb-hs1-pr100:v2.14.10-beta.1 -lc "cat /chatay-runtime-manifest.json; ls -lh /opt/resilientdb-hs1/bin"
```

## Boundary

This freezes a reproducible HS1 runtime image identity. The earlier warm-cluster
and cold-start behavior are evidenced in `v2.14.6-alpha.1` through
`v2.14.9-beta.1`; this version only closes packaging and image identity.
