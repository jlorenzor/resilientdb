# v2.18.3-alpha.1 Checklist

- [x] Add PBFT runtime Dockerfile with `/opt/resilientdb-pbft/bin`.
- [x] Add PBFT runtime image build script.
- [x] Add lightweight `benchmark/protocols/pbft:kv_service` with `MemoryDB`.
- [ ] Build the lightweight PBFT KV target and shared KV/config tools.
- [ ] Build `chatay-resilientdb-pbft:v2.18.3-alpha.1`.
- [ ] Verify runtime content.
- [ ] Update image inventory expectations for later benchmark gates.
- [ ] Commit, tag and push.
