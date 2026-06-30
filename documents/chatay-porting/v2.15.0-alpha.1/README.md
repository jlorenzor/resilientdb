# v2.15.0-alpha.1 - HS2 Fork Port Start

## Status

Closed by commit `35909ce0900436ad41a49353920e3387b9958e71`.

## Scope

This version starts the clean HS2 port inside the ResilientDB fork. The port is
not a Node.js side prototype; it is C++/Bazel code under:

```txt
platform/consensus/ordering/hs2
benchmark/protocols/hs2
proto/hs2.proto
```

## Result

The HS2 code path was applied as a fork-native batch because the exported patch
files required hunk recounting and one manual adaptation to the current
ResilientDB API. The implementation is now versioned in the fork and can be
built with Bazel.

## Boundary

This alpha starts the HS2 implementation track. It does not claim final
literature-level HotStuff-2 conformance.
