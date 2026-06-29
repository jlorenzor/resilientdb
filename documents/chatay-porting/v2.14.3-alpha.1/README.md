# ResilientDB Layout Map - v2.14.3-alpha.1

## Purpose

This version maps the current ResilientDB fork layout before applying HS1/PR100
or HS2 patches.

The previous package, `v2.14.2-alpha.1`, preserved the patch set. This package
answers whether those patches target files that already exist, create new
protocol trees, or need path reconciliation.

## Status

```txt
CLOSED_ALPHA_LAYOUT_MAP
```

## Main Findings

1. Current upstream ResilientDB contains PBFT, GeoPBFT, PoC and PoE consensus
   paths.
2. Current upstream ResilientDB does not contain `hotstuff` or `hs2` consensus
   directories.
3. HS1/PR100 must first recreate or port the `hotstuff` tree before runtime
   hardening patches can apply.
4. HS2 must first recreate `platform/consensus/ordering/hs2`,
   `benchmark/protocols/hs2`, and `proto/hs2.proto`.
5. Several older patches target paths such as
   `platform/consensus/ordering/common/response_manager.cpp`; in current
   upstream, the likely equivalent is under
   `platform/consensus/ordering/common/framework/response_manager.cpp`.
6. No Docker images are built in this version.

## Files In This Package

```txt
LAYOUT_MAP.md
PATCH_PATH_AUDIT.md
PORTING_DECISION.md
```

## Next Version

```txt
v2.14.4-alpha.1 - HS1/PR100 Baseline Tree Port
```

The next version should focus on HS1 first, not HS2:

1. recover/import the PR100 `platform/consensus/ordering/hotstuff` tree;
2. recover/import `benchmark/protocols/hotstuff`;
3. reconcile common framework paths;
4. verify Bazel query/build targets before applying cold-start repairs.

## Claim Boundary

This package is a source-map and patch-path audit only. It does not prove that
HS1 or HS2 builds in the current fork.
