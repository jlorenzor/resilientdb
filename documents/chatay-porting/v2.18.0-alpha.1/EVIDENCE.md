# v2.18.0-alpha.1 Evidence

## Build Command

Executed from the ResilientDB fork root:

```bash
bash tools/chatay/images/build_hs2_runtime_image.sh
```

## Image Summary

```json
{
  "schemaVersion": "chatay-consensus-runtime-image-summary-v1",
  "image": "chatay-resilientdb-hs2:v2.18.0-alpha.1",
  "imageId": "sha256:87d206e1a2ad9bc0d7af662479fa45e77da5f0c82238b67249450a6576bd1224",
  "sizeBytes": 976114081,
  "baseImage": "chatay-resilientdb-hs2:runtime-v2.13.20-alpha.1",
  "sourceCommit": "b7f265222bb91b97d10185e2cc0538826852485b",
  "sourceBranch": "consensus/hs2-runtime-image-v2.18.0-alpha.1"
}
```

The `sourceCommit` points to the runtime C++ source closed by
`v2.17.6-rc.1`. The `v2.18.0-alpha.1` commit records the packaging script,
Dockerfile, summary and evidence for rebuilding this image.

## Labels

```json
{
  "org.chatay.base_image": "chatay-resilientdb-hs2:runtime-v2.13.20-alpha.1",
  "org.chatay.consensus": "hs2",
  "org.chatay.image_role": "runtime-release",
  "org.chatay.semver": "v2.18.0-alpha.1",
  "org.chatay.source_branch": "consensus/hs2-runtime-image-v2.18.0-alpha.1",
  "org.chatay.source_commit": "b7f265222bb91b97d10185e2cc0538826852485b",
  "org.chatay.source_image": "chatay-resilientdb-hs2:runtime-v2.13.20-alpha.1"
}
```

## Runtime Content

```txt
bin/certificate_tools 6387816 bytes
bin/generate_region_config 20898 bytes
bin/key_generator_tools 5242136 bytes
bin/kv_service 9738208 bytes
bin/kv_service_tools 7618992 bytes
```

Content check:

```bash
docker run --rm --entrypoint sh chatay-resilientdb-hs2:v2.18.0-alpha.1 -lc "test -x /opt/resilientdb-hs2/bin/kv_service && test -x /opt/resilientdb-hs2/bin/kv_service_tools && test -x /usr/local/bin/chatay-hs2-kv_service && test -x /usr/local/bin/chatay-hs2-kv_service_tools && echo hs2-runtime-content-ok"
```

Result:

```txt
hs2-runtime-content-ok
```

## Windows/WSL Reproducibility Note

The build script copies Bazel outputs inside the Linux toolchain container.
This avoids Windows/WSL host permission problems with Bazel output symlinks.

## Boundary

This closes a packaging gate. It does not yet close the runtime-image smoke gate
or the PBFT/HS1/HS2 benchmark gate.
