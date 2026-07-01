# v2.18.4-beta.1 Checklist

- [x] Add cold-start matrix runner.
- [x] Reuse runtime-image KV runner to avoid duplicated protocol setup.
- [x] Run PBFT cold-start repetitions.
- [x] Run HS1/PR100 cold-start repetitions.
- [x] Run HS2 cold-start repetitions.
- [x] Separate container total duration from process readiness duration.
- [x] Separate operation duration for the SET/GET pair.
- [x] Detect and document ephemeral-port collision risk.
- [x] Move default cold-start ports outside the common Linux ephemeral range.
- [x] Store detailed logs under ignored `logs/`.
- [x] Persist summarized evidence outside ignored `logs/`.
