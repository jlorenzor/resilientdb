# Segment Schema

## CSV Columns

```csv
semver,runId,protocol,segment,status,durationMs,image,imageId,sizeBytes,command,notes
```

## Protocol Values

```txt
pbft
hs1-pr100
hs2
```

## Segment Values

```txt
image-check
build
cold-start
warm-cluster
phase-trace
```

## Status Values

```txt
PASSED
FAILED
PLANNED
UNSUPPORTED
```

## Methodological Rule

Only `PASSED` segments may be used as evidence. `PLANNED` and `UNSUPPORTED`
rows are roadmap evidence, not performance evidence.
