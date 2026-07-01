# v2.18.3-alpha.1 Evidence

## Command

```bash
bash tools/chatay/images/build_pbft_runtime_image.sh
```

## Expected Result

```txt
chatay-resilientdb-pbft:v2.18.3-alpha.1
```

The image must include:

```txt
/opt/resilientdb-pbft/bin/kv_service
/opt/resilientdb-pbft/bin/kv_service_tools
/opt/resilientdb-pbft/bin/certificate_tools
/opt/resilientdb-pbft/bin/key_generator_tools
/opt/resilientdb-pbft/bin/generate_region_config
```

## Build Note

The initial attempt with `//service/kv:kv_service` pulled DuckDB into the build
and was not suitable for the local benchmark runtime image. The corrected path
uses `//benchmark/protocols/pbft:kv_service` with `MemoryDB`, matching the
lightweight HS1/HS2 benchmark layout.

## Current Blocker

The first Docker/Bazel attempt was interrupted by local resource/runtime failure
while compiling DuckDB:

```txt
fatal error: unexpected signal during runtime execution
signal SIGBUS: bus error
```

After that failure Docker Desktop stopped exposing the daemon pipe to the CLI:

```txt
open //./pipe/docker_engine: The system cannot find the file specified
```

The PBFT build path has been corrected to avoid DuckDB, but the image is not
closed until Docker is healthy and the corrected script builds successfully.

## Closed Result

Docker was restarted and the corrected lightweight path succeeded:

```txt
image=chatay-resilientdb-pbft:v2.18.3-alpha.1
imageId=sha256:47c138d18e5061c6eb7461e876f2cc8bbd6ca8af29bf6ed18376732da059aa4a
sizeBytes=1070001534
contentCheck=pbft-runtime-content-ok
```

The runtime image includes:

```txt
/opt/resilientdb-pbft/bin/kv_service
/opt/resilientdb-pbft/bin/kv_service_tools
/opt/resilientdb-pbft/bin/certificate_tools
/opt/resilientdb-pbft/bin/key_generator_tools
/opt/resilientdb-pbft/bin/generate_region_config
```
