# v2.14.7-alpha.1 Evidence - HS1 Cold-Start Instrumentation

## Status

Pending final post-commit validation.

## Expected Evidence

The final validation run must include:

```txt
status=RUNTIME_SMOKE_PASSED
operationCount=1
passedOperations=1
```

The node logs must contain `CHATAY_HS1_COLD_START` events for:

- process entry;
- config load;
- private key load;
- certificate load;
- consensus manager constructor;
- service network constructor;
- heartbeat send/receive;
- readiness reached;
- commitment init;
- NEWVIEW send.

## Raw Evidence Location

Raw logs are generated under:

```txt
documents/chatay-porting/v2.14.7-alpha.1/logs/<run-id>
```

The raw logs are intentionally ignored because they contain ephemeral key and
certificate files generated for the local run.
