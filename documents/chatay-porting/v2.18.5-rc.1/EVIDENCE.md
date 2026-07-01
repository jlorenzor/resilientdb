# v2.18.5-rc.1 Evidence

## Extracted Events

The extractor produced 2862 trace events plus the CSV header.

| Protocol | Events extracted |
| --- | ---: |
| PBFT | 753 |
| HS1/PR100 | 1317 |
| HS2 | 792 |

## Observed Markers

PBFT includes local startup/readiness markers and commit markers such as:

```txt
message_manager.cpp:188] seq:... has been committed
```

HS1/PR100 includes local startup/readiness markers, `consensus_commit`, and
HotStuff-style vote markers:

```txt
TYPE_PREPARE_VOTE
TYPE_PRECOMMIT_VOTE
TYPE_COMMIT_VOTE
```

HS2 includes local startup/readiness markers and experimental certification
markers:

```txt
CHATAY_HS2_PIPELINE certified ... phase2_voters=3
```

All three protocols include client-visible SET/GET success markers from the KV
tools.

## Explicit Non-Coverage

This release candidate does not claim view-change coverage. The source run did
not inject leader failure, replica failure or network partition.
