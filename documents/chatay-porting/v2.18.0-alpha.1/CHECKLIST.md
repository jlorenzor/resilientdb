# v2.18.0-alpha.1 Checklist

| Item | Status | Evidence |
| --- | --- | --- |
| Create HS2 runtime Dockerfile. | Done | `tools/chatay/images/Dockerfile.hs2-runtime`. |
| Create reproducible image build script. | Done | `tools/chatay/images/build_hs2_runtime_image.sh`. |
| Build HS2 runtime binaries through Docker/Bazel. | Done | `logs/bazel-build.log` local ignored log. |
| Copy only runtime binaries into image context. | Done | `runtime-content.txt`. |
| Build differentiated image `chatay-resilientdb-hs2:v2.18.0-alpha.1`. | Done | `image-summary.json`. |
| Keep image close to 0.9-1.1 GB policy. | Done | `976114081` bytes. |
| Verify runtime files and command symlinks exist. | Done | `hs2-runtime-content-ok`. |
| Avoid benchmark claims. | Done | README and evidence delimit this as packaging only. |
