# v2.18.3-beta.1 Checklist

- [x] Add runtime-image warm-cluster runner.
- [x] Avoid build steps during runtime measurement.
- [x] Use frozen PBFT, HS1 and HS2 images.
- [x] Generate local keys and certificates inside the runtime container.
- [x] Generate ResilientDB-compatible server and client configs without Bazel.
- [x] Start four consensus replicas and one client/gateway process.
- [x] Validate PBFT SET/GET path.
- [x] Validate HS1/PR100 SET/GET path.
- [x] Validate HS2 SET/GET path.
- [x] Store per-protocol manifests and operations CSV under ignored logs.
- [x] Persist summarized evidence outside ignored logs.
- [x] Preserve explicit limitation: local runtime warm-cluster only.
