# v2.14.5-alpha.2 - Professional Branch Naming

## Objective

Normalize branch names in the ResilientDB fork to avoid the `codex/*` pattern
in research-visible branches.

## New namespace

Use:

```txt
consensus/*
```

for all fork branches related to PBFT, HS1, HS2, Bazel builds, runtime images,
and consensus experimentation.

## Branch aliases pushed

```txt
consensus/hs2-port-v2.14.1-alpha.1
consensus/hs1-hs2-patch-export-v2.14.2-alpha.1
consensus/layout-map-v2.14.3-alpha.1
consensus/hs1-baseline-tree-v2.14.4-alpha.1
consensus/hs1-bazel-reconcile-v2.14.5-alpha.1
```

## Current branch

```txt
consensus/hs1-bazel-reconcile-v2.14.5-alpha.1
```

## Policy

Do not create new `codex/*` branches in this fork.

Remote legacy branches with the old prefix were not deleted in this step.
Deleting them should be an explicit cleanup operation after confirming no open
review, note, script, or external reference depends on them.
