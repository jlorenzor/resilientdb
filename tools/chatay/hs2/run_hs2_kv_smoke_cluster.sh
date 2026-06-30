#!/usr/bin/env bash

set -Eeuo pipefail

VERSION="${CHATAY_SEMVER:-v2.15.6-beta.1}"
REPLICA_COUNT="${HS2_REPLICA_COUNT:-4}"
CLIENT_PROCESS_COUNT="${HS2_CLIENT_PROCESS_COUNT:-1}"
BASE_PORT="${HS2_BASE_PORT:-24000}"
READY_TIMEOUT_SEC="${HS2_READY_TIMEOUT_SEC:-90}"
CLIENT_TIMEOUT_SEC="${HS2_CLIENT_TIMEOUT_SEC:-45}"
START_STAGGER_SEC="${HS2_START_STAGGER_SEC:-0.5}"
AFTER_SET_SLEEP_SEC="${HS2_AFTER_SET_SLEEP_SEC:-2}"
BAZEL_JOBS="${BAZEL_JOBS:-4}"
OPERATION_COUNT="${HS2_OPERATION_COUNT:-1}"
RUN_ID="${RUN_ID:-$(date -u +%Y%m%dT%H%M%SZ)-hs2-kv}"
STOP_NODE_AFTER_READY="${HS2_STOP_NODE_AFTER_READY:-0}"
STOP_NODE_AFTER_READY_DELAY_SEC="${HS2_STOP_NODE_AFTER_READY_DELAY_SEC:-1}"
SLOW_NODE_ID="${HS2_SLOW_NODE_ID:-0}"
SLOW_NODE_DELAY_SEC="${HS2_SLOW_NODE_DELAY_SEC:-0}"

ROOT="$(git rev-parse --show-toplevel)"
TOTAL_PROCESS_COUNT=$((REPLICA_COUNT + CLIENT_PROCESS_COUNT))
LOG_ROOT="${CHATAY_LOG_ROOT:-${ROOT}/documents/chatay-porting/${VERSION}/logs/${RUN_ID}}"
LOG_PARENT="$(dirname "${LOG_ROOT}")"
CONFIG_ROOT="${LOG_ROOT}/config"
CERT_ROOT="${LOG_ROOT}/cert"
SERVER_CONFIG="${CONFIG_ROOT}/server/server.config"
CLIENT_CONFIG="${CONFIG_ROOT}/interface/service.config"
SERVER_BIN="${ROOT}/bazel-bin/benchmark/protocols/hs2/kv_service"
CLIENT_BIN="${ROOT}/bazel-bin/service/tools/kv/api_tools/kv_service_tools"
TEST_KEY="${HS2_TEST_KEY:-chatay-hs2-${RUN_ID}}"
TEST_VALUE="${HS2_TEST_VALUE:-chatay-hs2-value-${RUN_ID}}"

pids=()
result_status="FAILED"
error_code=""
passed_operations=0

log() {
  printf '[%s] %s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$*"
}

run_lf_script() {
  local script_path="$1"
  shift
  local normalized_script="${LOG_ROOT}/$(basename "${script_path}").lf.sh"
  sed 's/\r$//' "${ROOT}/${script_path}" > "${normalized_script}"
  bash "${normalized_script}" "$@"
}

write_manifest() {
  local finished_at branch commit
  finished_at="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  branch="$(git -C "${ROOT}" rev-parse --abbrev-ref HEAD 2>/dev/null || true)"
  commit="$(git -C "${ROOT}" rev-parse HEAD 2>/dev/null || true)"

  cat > "${LOG_ROOT}/manifest.json" <<EOF
{
  "semver": "${VERSION}",
  "runId": "${RUN_ID}",
  "protocol": "hs2",
  "status": "${result_status}",
  "errorCode": "${error_code}",
  "branch": "${branch}",
  "commit": "${commit}",
  "replicaCount": ${REPLICA_COUNT},
  "clientProcessCount": ${CLIENT_PROCESS_COUNT},
  "totalProcessCount": ${TOTAL_PROCESS_COUNT},
  "basePort": ${BASE_PORT},
  "readyTimeoutSec": ${READY_TIMEOUT_SEC},
  "clientTimeoutSec": ${CLIENT_TIMEOUT_SEC},
  "operationCount": ${OPERATION_COUNT},
  "passedOperations": ${passed_operations},
  "stopNodeAfterReady": ${STOP_NODE_AFTER_READY},
  "stopNodeAfterReadyDelaySec": ${STOP_NODE_AFTER_READY_DELAY_SEC},
  "slowNodeId": ${SLOW_NODE_ID},
  "slowNodeDelaySec": ${SLOW_NODE_DELAY_SEC},
  "testKey": "${TEST_KEY}",
  "testValue": "${TEST_VALUE}",
  "serverConfig": "${SERVER_CONFIG}",
  "clientConfig": "${CLIENT_CONFIG}",
  "finishedAt": "${finished_at}"
}
EOF
}

cleanup() {
  if [[ "${HS2_KEEP_CLUSTER:-0}" == "1" ]]; then
    log "HS2_KEEP_CLUSTER=1; leaving ${#pids[@]} processes running"
    write_manifest || true
    return
  fi
  for pid in "${pids[@]:-}"; do
    if kill -0 "${pid}" >/dev/null 2>&1; then
      kill "${pid}" >/dev/null 2>&1 || true
    fi
  done
  wait "${pids[@]:-}" >/dev/null 2>&1 || true
  write_manifest || true
}
trap cleanup EXIT

mkdir -p "${LOG_ROOT}" "${CONFIG_ROOT}/server" "${CONFIG_ROOT}/interface" "${CERT_ROOT}"
if [[ ! -f "${LOG_PARENT}/.gitignore" ]]; then
  {
    printf '*\n'
    printf '!.gitignore\n'
  } > "${LOG_PARENT}/.gitignore"
fi

log "HS2 KV smoke-cluster run ${RUN_ID}"
log "repo=${ROOT}"
log "replicas=${REPLICA_COUNT} client_processes=${CLIENT_PROCESS_COUNT} base_port=${BASE_PORT}"
log "operation_count=${OPERATION_COUNT} client_timeout_sec=${CLIENT_TIMEOUT_SEC}"
log "fault knobs stop_node_after_ready=${STOP_NODE_AFTER_READY} slow_node_id=${SLOW_NODE_ID}"

(
  cd "${ROOT}"
  bazel build --jobs="${BAZEL_JOBS}" \
    //benchmark/protocols/hs2:kv_service \
    //service/tools/kv/api_tools:kv_service_tools \
    //tools:certificate_tools \
    //tools:key_generator_tools \
    //tools:generate_region_config
) 2>&1 | tee "${LOG_ROOT}/bazel-build.log"

iplist=()
for _ in $(seq 1 "${TOTAL_PROCESS_COUNT}"); do
  iplist+=("127.0.0.1")
done

(
  cd "${ROOT}"
  run_lf_script service/tools/config/generate_keys_and_certs.sh \
    "${ROOT}" "${CERT_ROOT}" "${CERT_ROOT}" "${BASE_PORT}" \
    "${CLIENT_PROCESS_COUNT}" "${iplist[@]}"
) 2>&1 | tee "${LOG_ROOT}/generate-keys.log" || {
  error_code="GENERATE_KEYS_FAILED"
  log "ERROR: key/certificate generation failed"
  exit 10
}

(
  cd "${ROOT}"
  run_lf_script service/tools/config/generate_config.sh \
    "${ROOT}" "${CERT_ROOT}" "${CERT_ROOT}" "${CONFIG_ROOT}" "${CERT_ROOT}" \
    "${CLIENT_PROCESS_COUNT}" "${BASE_PORT}" "${iplist[@]}"
) 2>&1 | tee "${LOG_ROOT}/generate-config.log" || {
  error_code="GENERATE_CONFIG_FAILED"
  log "ERROR: config generation failed"
  exit 11
}

if [[ ! -x "${SERVER_BIN}" ]]; then
  error_code="SERVER_BIN_NOT_FOUND"
  log "ERROR: ${SERVER_BIN} is not executable"
  exit 20
fi

if [[ ! -x "${CLIENT_BIN}" ]]; then
  error_code="CLIENT_BIN_NOT_FOUND"
  log "ERROR: ${CLIENT_BIN} is not executable"
  exit 21
fi

if [[ ! -f "${SERVER_CONFIG}" || ! -f "${CLIENT_CONFIG}" ]]; then
  error_code="CONFIG_NOT_FOUND"
  log "ERROR: generated config files are missing"
  exit 22
fi

cat > "${CLIENT_CONFIG}" <<EOF
{
  "replica_info": [
    {
      "id": ${TOTAL_PROCESS_COUNT},
      "ip": "127.0.0.1",
      "port": $((BASE_PORT + TOTAL_PROCESS_COUNT))
    }
  ],
  "region_id": 1
}
EOF

log "starting ${TOTAL_PROCESS_COUNT} hs2 kv_service processes"
for i in $(seq 1 "${TOTAL_PROCESS_COUNT}"); do
  if [[ "${SLOW_NODE_ID}" -eq "${i}" && "${SLOW_NODE_DELAY_SEC}" != "0" ]]; then
    log "delaying node-${i} startup by ${SLOW_NODE_DELAY_SEC}s"
    sleep "${SLOW_NODE_DELAY_SEC}"
  fi
  node_log="${LOG_ROOT}/node-${i}.log"
  "${SERVER_BIN}" "${SERVER_CONFIG}" "${CERT_ROOT}/node${i}.key.pri" \
    "${CERT_ROOT}/cert_${i}.cert" > "${node_log}" 2>&1 &
  pid="$!"
  pids+=("${pid}")
  printf '%s\n' "${pid}" > "${LOG_ROOT}/node-${i}.pid"
  log "node-${i} pid=${pid} log=${node_log}"
  sleep "${START_STAGGER_SEC}"
done

deadline=$((SECONDS + READY_TIMEOUT_SEC))
ready_count=0
while (( SECONDS < deadline )); do
  ready_count=0
  for i in $(seq 1 "${TOTAL_PROCESS_COUNT}"); do
    if grep -q "Server ${i} is ready" "${LOG_ROOT}/node-${i}.log" 2>/dev/null; then
      ready_count=$((ready_count + 1))
    fi
  done

  printf '%s ready=%s/%s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
    "${ready_count}" "${TOTAL_PROCESS_COUNT}" >> "${LOG_ROOT}/readiness.trace"

  if [[ "${ready_count}" -eq "${TOTAL_PROCESS_COUNT}" ]]; then
    log "all processes ready (${ready_count}/${TOTAL_PROCESS_COUNT})"
    break
  fi

  for pid in "${pids[@]}"; do
    if ! kill -0 "${pid}" >/dev/null 2>&1; then
      error_code="PROCESS_EXITED_BEFORE_READY"
      log "ERROR: process ${pid} exited before readiness"
      exit 30
    fi
  done
  sleep 1
done

if [[ "${ready_count}" -ne "${TOTAL_PROCESS_COUNT}" ]]; then
  error_code="READINESS_TIMEOUT"
  log "ERROR: readiness timeout (${ready_count}/${TOTAL_PROCESS_COUNT})"
  exit 31
fi

if [[ "${STOP_NODE_AFTER_READY}" -gt 0 ]]; then
  if [[ "${STOP_NODE_AFTER_READY}" -gt "${TOTAL_PROCESS_COUNT}" ]]; then
    error_code="STOP_NODE_OUT_OF_RANGE"
    log "ERROR: stop node ${STOP_NODE_AFTER_READY} is out of range"
    exit 32
  fi
  log "stopping node-${STOP_NODE_AFTER_READY} after readiness in ${STOP_NODE_AFTER_READY_DELAY_SEC}s"
  sleep "${STOP_NODE_AFTER_READY_DELAY_SEC}"
  stop_pid="${pids[$((STOP_NODE_AFTER_READY - 1))]}"
  if kill -0 "${stop_pid}" >/dev/null 2>&1; then
    kill "${stop_pid}" >/dev/null 2>&1 || true
    printf '%s stopped node=%s pid=%s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
      "${STOP_NODE_AFTER_READY}" "${stop_pid}" >> "${LOG_ROOT}/fault.trace"
  else
    error_code="STOP_NODE_ALREADY_EXITED"
    log "ERROR: node-${STOP_NODE_AFTER_READY} already exited before fault injection"
    exit 33
  fi
fi

for operation_index in $(seq 1 "${OPERATION_COUNT}"); do
  op_label="$(printf '%03d' "${operation_index}")"
  op_key="${TEST_KEY}-${op_label}"
  op_value="${TEST_VALUE}-${op_label}"
  set_log="${LOG_ROOT}/client-set-${op_label}.log"
  get_log="${LOG_ROOT}/client-get-${op_label}.log"

  log "operation ${operation_index}/${OPERATION_COUNT}: KV SET timeout=${CLIENT_TIMEOUT_SEC}s"
  set +e
  timeout "${CLIENT_TIMEOUT_SEC}" "${CLIENT_BIN}" "${CLIENT_CONFIG}" set \
    "${op_key}" "${op_value}" > "${set_log}" 2>&1
  client_set_rc=$?
  set -e
  if [[ "${client_set_rc}" -eq 124 ]]; then
    error_code="CLIENT_SET_TIMEOUT_OP_${op_label}"
    log "ERROR: client SET operation ${op_label} timed out after ${CLIENT_TIMEOUT_SEC}s"
    exit 40
  fi
  if [[ "${client_set_rc}" -ne 0 ]]; then
    error_code="CLIENT_SET_FAILED_OP_${op_label}"
    log "ERROR: client SET operation ${op_label} failed rc=${client_set_rc}"
    exit 40
  fi

  if ! grep -Eq "ret = 0|client set ret = 0" "${set_log}"; then
    error_code="CLIENT_SET_NONZERO_OP_${op_label}"
    log "ERROR: client SET operation ${op_label} did not report ret=0"
    exit 41
  fi

  sleep "${AFTER_SET_SLEEP_SEC}"

  log "operation ${operation_index}/${OPERATION_COUNT}: KV GET timeout=${CLIENT_TIMEOUT_SEC}s"
  set +e
  timeout "${CLIENT_TIMEOUT_SEC}" "${CLIENT_BIN}" "${CLIENT_CONFIG}" get \
    "${op_key}" > "${get_log}" 2>&1
  client_get_rc=$?
  set -e
  if [[ "${client_get_rc}" -eq 124 ]]; then
    error_code="CLIENT_GET_TIMEOUT_OP_${op_label}"
    log "ERROR: client GET operation ${op_label} timed out after ${CLIENT_TIMEOUT_SEC}s"
    exit 42
  fi
  if [[ "${client_get_rc}" -ne 0 ]]; then
    error_code="CLIENT_GET_FAILED_OP_${op_label}"
    log "ERROR: client GET operation ${op_label} failed rc=${client_get_rc}"
    exit 42
  fi

  if ! grep -Fq "${op_value}" "${get_log}"; then
    error_code="CLIENT_GET_VALUE_MISMATCH_OP_${op_label}"
    log "ERROR: client GET operation ${op_label} did not return expected value"
    exit 43
  fi

  passed_operations="${operation_index}"
done

result_status="RUNTIME_SMOKE_PASSED"
error_code=""
log "HS2 KV smoke-cluster passed operations=${passed_operations}/${OPERATION_COUNT}"
