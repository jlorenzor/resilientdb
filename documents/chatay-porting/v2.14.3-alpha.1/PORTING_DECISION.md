# Porting Decision - v2.14.3-alpha.1

## Decision

Proceed with HS1 before HS2.

## Rationale

HS1/PR100 is the methodological bridge between upstream PBFT and the target HS2
research implementation. Since the current fork does not contain HotStuff or
HS2 trees, porting HS1 first reduces risk:

1. it confirms that a non-PBFT consensus module can be restored inside the
   current ResilientDB layout;
2. it exercises the same Bazel/service/KV integration surfaces needed by HS2;
3. it allows focused correction of the known HS1 cold-start/readiness issue;
4. it avoids benchmarking HS2 before the intermediate HotStuff-like baseline is
   reproducible inside the fork.

## Version Path

| Version | Scope |
| --- | --- |
| `v2.14.3-alpha.1` | Map layout and classify patch paths. |
| `v2.14.4-alpha.1` | Recover/import HS1/PR100 baseline tree. |
| `v2.14.5-alpha.1` | Validate HS1 Bazel query/build path. |
| `v2.14.6-alpha.1` | Reproduce HS1 KV warm-cluster path from fork-produced artifacts. |
| `v2.14.7-alpha.1` | Instrument HS1 cold-start/readiness. |
| `v2.14.8-alpha.1` | Parameterize/correct HS1 NEWVIEW bootstrap timing. |
| `v2.14.9-beta.1` | Validate HS1 cold-start improvement over multiple runs. |
| `v2.14.10-beta.1` | Freeze HS1 runtime image within size policy. |
| `v2.14.11-rc.1` | Produce HS1 conformance report against HotStuff literature and PR100. |
| `v2.15.0-alpha.1` | Begin fork-native HS2 implementation after HS1 baseline closure. |

## Image Constraint

No image is built in this version.

Future images must remain within the policy from `v2.14.2-alpha.1`:

```txt
expected runtime image size: 0.9 GB - 1.1 GB
review threshold: > 1.25 GB
```

If an image exceeds the threshold, optimize the Dockerfile before using it for
benchmarks.

## Claim Boundary

This decision does not claim HS1 or HS2 is implemented in the current fork yet.
It only establishes the order and technical surfaces for the implementation
work.
