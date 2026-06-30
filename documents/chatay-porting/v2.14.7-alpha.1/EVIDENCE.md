# v2.14.7-alpha.1 Evidence - HS1 Cold-Start Instrumentation

## Result

`v2.14.7-alpha.1` passed the HS1 cold-start instrumentation smoke gate from the
real ResilientDB fork.

```txt
runId=20260630T043005Z-hs1-kv
status=RUNTIME_SMOKE_PASSED
branch=consensus/hs1-cold-start-v2.14.7-alpha.1
commit=0ff1429367593570f7d5a893575e8aa7738a376c
operationCount=1
passedOperations=1
```

## Trace Evidence

The final validation run confirmed `CHATAY_HS1_COLD_START` events for:

- `process_main_enter`;
- `read_config_finish`;
- `read_private_key_finish`;
- `read_cert_finish`;
- `consensus_manager_ctor_finish`;
- `service_network_ctor_finish`;
- `heartbeat_send_start`;
- `heartbeat_receive_enter`;
- `readiness_reached`;
- `commitment_init_enter`;
- `send_newview_attempt`.

`node-1.log` contained 119 `CHATAY_HS1_COLD_START` trace lines in the validated
run.

## Raw Evidence Location

Raw logs are generated under:

```txt
documents/chatay-porting/v2.14.7-alpha.1/logs/20260630T043005Z-hs1-kv
```

The raw logs are intentionally ignored because they contain ephemeral key and
certificate files generated for the local run.
