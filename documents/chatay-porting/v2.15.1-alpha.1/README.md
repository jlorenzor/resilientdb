# v2.15.1-alpha.1 - HS2 C++/Bazel Skeleton

## Status

Closed.

## Implemented

- `Hs2Consensus` C++ class.
- `Hs2Block`, `Hs2Vote`, `Hs2QuorumCertificate` and phase model.
- Bazel target `//benchmark/protocols/hs2:hs2_skeleton_probe`.

## Validation

```txt
bazel build //benchmark/protocols/hs2:hs2_skeleton_probe
./bazel-bin/benchmark/protocols/hs2/hs2_skeleton_probe
hs2 skeleton ok
```

## Boundary

This is a structural skeleton and type gate. It does not exercise the KV
network path.
