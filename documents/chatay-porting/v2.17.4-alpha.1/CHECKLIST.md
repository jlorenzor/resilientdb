# v2.17.4-alpha.1 Checklist

- [x] Add stop-node-after-readiness knob to smoke runner.
- [x] Add slow-start node knob to smoke runner.
- [x] Add fault matrix runner.
- [x] Record summary CSV for scenarios.
- [x] Run baseline scenario.
- [x] Run stopped non-leader scenario.
- [x] Run slow-start scenario.
- [x] Run stopped leader scenario as classified evidence.

## Deferred

- [ ] Add real network delay/packet loss injection.
- [ ] Add leader equivocation tests.
- [ ] Add internal HS2 phase traces proving proposal/vote/new-view causality.
- [ ] Require stopped-leader as a formal pass/fail gate after custom HS2
      consensus messages are fully networked.
