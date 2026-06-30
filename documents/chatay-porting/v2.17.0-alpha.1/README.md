# v2.17.0-alpha.1 - HS2 Paper-to-Code Matrix

## Status

Closed as an alpha audit package.

## Scope

This version does not change runtime code. It freezes the current HS2
implementation map before hardening starts in `v2.17.1-alpha.1`.

## Main Artifact

- [PAPER_TO_CODE_MATRIX.md](PAPER_TO_CODE_MATRIX.md)

## Decision

HS2 is allowed to continue into hardening, but not into benchmark.

The next code version must address the `TYPE_NEW_TXNS` bypass through
`MessageManagerBasic`.

## Next Version

```txt
v2.17.1-alpha.1 - HS2 TYPE_NEW_TXNS pipeline hardening
```
