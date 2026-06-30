# v2.14.5-alpha.3 - Legacy Branch Cleanup

## Objective

Prune remote `codex/*` branches after professional aliases were created under
`consensus/*`.

## Status

```txt
CLOSED_ALPHA
```

## Validation

Open pull requests were checked before deletion. No open PRs used a `codex/*`
head branch.

## Remaining branches

```txt
consensus/hs2-port-v2.14.1-alpha.1
consensus/hs1-hs2-patch-export-v2.14.2-alpha.1
consensus/layout-map-v2.14.3-alpha.1
consensus/hs1-baseline-tree-v2.14.4-alpha.1
consensus/hs1-bazel-reconcile-v2.14.5-alpha.1
```

## Policy

Do not create new `codex/*` branches.

Use `consensus/*` for ResilientDB consensus work.
