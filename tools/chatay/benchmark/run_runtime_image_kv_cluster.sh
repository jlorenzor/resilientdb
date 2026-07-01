#!/usr/bin/env bash

set -Eeuo pipefail

VERSION="${CHATAY_SEMVER:-v2.18.3-beta.1}"
RUN_ID="${RUN_ID:-$(date -u +%Y%m%dT%H%M%SZ)-runtime-warm}"
ROOT="$(git rev-parse --show-toplevel)"
VERSION_DIR="${ROOT}/documents/chatay-porting/${VERSION}"
LOG_ROOT="${CHATAY_LOG_ROOT:-${VERSION_DIR}/logs/${RUN_ID}}"
SUMMARY_CSV="${LOG_ROOT}/runtime-warm-summary.csv"
SUMMARY_MD="${LOG_ROOT}/runtime-warm-summary.md"

PROTOCOLS="${CHATAY_PROTOCOLS:-pbft,hs1-pr100,hs2}"
OPERATION_COUNT="${CHATAY_OPERATION_COUNT:-10}"
READY_TIMEOUT_SEC="${CHATAY_READY_TIMEOUT_SEC:-90}"
CLIENT_TIMEOUT_SEC="${CHATAY_CLIENT_TIMEOUT_SEC:-45}"
START_STAGGER_SEC="${CHATAY_START_STAGGER_SEC:-0.5}"
AFTER_SET_SLEEP_SEC="${CHATAY_AFTER_SET_SLEEP_SEC:-1}"
BASE_PORT_START="${CHATAY_BASE_PORT_START:-31000}"
PORT_STRIDE="${CHATAY_PORT_STRIDE:-100}"

PBFT_IMAGE="${CHATAY_PBFT_IMAGE:-chatay-resilientdb-pbft:v2.18.3-alpha.1}"
HS1_IMAGE="${CHATAY_HS1_IMAGE:-chatay-resilientdb-hs1-pr100:v2.14.10-beta.1}"
HS2_IMAGE="${CHATAY_HS2_IMAGE:-chatay-resilientdb-hs2:v2.18.0-alpha.1}"

mkdir -p "${LOG_ROOT}" "${VERSION_DIR}/logs"
if [[ ! -f "${VERSION_DIR}/logs/.gitignore" ]]; then
  printf '*\n!.gitignore\n' > "${VERSION_DIR}/logs/.gitignore"
fi

now_ms() {
  date -u +%s%3N
}

protocol_image() {
  case "$1" in
    pbft) printf '%s' "${PBFT_IMAGE}" ;;
    hs1-pr100) printf '%s' "${HS1_IMAGE}" ;;
    hs2) printf '%s' "${HS2_IMAGE}" ;;
    *) printf 'unknown protocol: %s\n' "$1" >&2; return 2 ;;
  esac
}

protocol_bin_root() {
  case "$1" in
    pbft) printf '/opt/resilientdb-pbft/bin' ;;
    hs1-pr100) printf '/opt/resilientdb-hs1/bin' ;;
    hs2) printf '/opt/resilientdb-hs2/bin' ;;
    *) printf 'unknown protocol: %s\n' "$1" >&2; return 2 ;;
  esac
}

container_name_for() {
  local protocol="$1"
  printf 'chatay-%s-%s' "${protocol//[^a-zA-Z0-9]/-}" "${RUN_ID//[^a-zA-Z0-9]/-}"
}

write_payload() {
  local payload="$1"
  cat > "${payload}" <<'PAYLOAD'
#!/usr/bin/env bash
set -Eeuo pipefail

PROTOCOL="${PROTOCOL:?}"
BIN_ROOT="${BIN_ROOT:?}"
BASE_PORT="${BASE_PORT:?}"
OPERATION_COUNT="${OPERATION_COUNT:?}"
READY_TIMEOUT_SEC="${READY_TIMEOUT_SEC:?}"
CLIENT_TIMEOUT_SEC="${CLIENT_TIMEOUT_SEC:?}"
START_STAGGER_SEC="${START_STAGGER_SEC:?}"
AFTER_SET_SLEEP_SEC="${AFTER_SET_SLEEP_SEC:?}"
RUN_ID="${RUN_ID:?}"

REPLICA_COUNT=4
CLIENT_PROCESS_COUNT=1
TOTAL_PROCESS_COUNT=$((REPLICA_COUNT + CLIENT_PROCESS_COUNT))
WORK_ROOT="/work"
CONFIG_ROOT="${WORK_ROOT}/config"
CERT_ROOT="${WORK_ROOT}/cert"
SERVER_CONFIG="${CONFIG_ROOT}/server/server.config"
CLIENT_CONFIG="${CONFIG_ROOT}/interface/service.config"
SERVER_BIN="${BIN_ROOT}/kv_service"
CLIENT_BIN="${BIN_ROOT}/kv_service_tools"
KEYGEN_BIN="${BIN_ROOT}/key_generator_tools"
CERT_BIN="${BIN_ROOT}/certificate_tools"
TEST_KEY="chatay-${PROTOCOL}-${RUN_ID}"
TEST_VALUE="chatay-${PROTOCOL}-value-${RUN_ID}"

pids=()
result_status="FAILED"
error_code=""
ready_count=0
passed_operations=0
ready_duration_ms=0
operations_duration_ms=0
total_duration_ms=0

now_ms() { date -u +%s%3N; }

log() {
  printf '[%s] [%s] %s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "${PROTOCOL}" "$*" | tee -a "${WORK_ROOT}/run.log"
}

write_manifest() {
  cat > "${WORK_ROOT}/manifest.json" <<EOF
{
  "schemaVersion": "chatay-runtime-image-kv-cluster-v1",
  "protocol": "${PROTOCOL}",
  "runId": "${RUN_ID}",
  "status": "${result_status}",
  "errorCode": "${error_code}",
  "replicaCount": ${REPLICA_COUNT},
  "clientProcessCount": ${CLIENT_PROCESS_COUNT},
  "totalProcessCount": ${TOTAL_PROCESS_COUNT},
  "basePort": ${BASE_PORT},
  "readyTimeoutSec": ${READY_TIMEOUT_SEC},
  "clientTimeoutSec": ${CLIENT_TIMEOUT_SEC},
  "operationCount": ${OPERATION_COUNT},
  "passedOperations": ${passed_operations},
  "readyCount": ${ready_count},
  "readyDurationMs": ${ready_duration_ms},
  "operationsDurationMs": ${operations_duration_ms},
  "totalDurationMs": ${total_duration_ms},
  "finishedAt": "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
}
EOF
}

cleanup() {
  for pid in "${pids[@]:-}"; do
    if kill -0 "${pid}" >/dev/null 2>&1; then
      kill "${pid}" >/dev/null 2>&1 || true
    fi
  done
  wait "${pids[@]:-}" >/dev/null 2>&1 || true
  write_manifest || true
}
trap cleanup EXIT

start_ms="$(now_ms)"
mkdir -p "${CONFIG_ROOT}/server" "${CONFIG_ROOT}/interface" "${CERT_ROOT}"

for required in "${SERVER_BIN}" "${CLIENT_BIN}" "${KEYGEN_BIN}" "${CERT_BIN}"; do
  if [[ ! -x "${required}" ]]; then
    error_code="REQUIRED_BINARY_MISSING"
    log "ERROR: missing executable ${required}"
    exit 20
  fi
done

log "generating keys and certificates"
"${KEYGEN_BIN}" "${CERT_ROOT}/admin" >> "${WORK_ROOT}/keygen.log" 2>&1
for i in $(seq 1 "${TOTAL_PROCESS_COUNT}"); do
  port=$((BASE_PORT + i))
  node_type="replica"
  if [[ "${i}" -gt "${REPLICA_COUNT}" ]]; then
    node_type="client"
  fi
  "${KEYGEN_BIN}" "${CERT_ROOT}/node${i}" >> "${WORK_ROOT}/keygen.log" 2>&1
  "${CERT_BIN}" "${CERT_ROOT}" "${CERT_ROOT}/admin.key.pri" "${CERT_ROOT}/admin.key.pub" \
    "${CERT_ROOT}/node${i}.key.pub" "${i}" "127.0.0.1" "${port}" "${node_type}" \
    >> "${WORK_ROOT}/certificate.log" 2>&1
done

log "writing configs"
{
  printf '{\n  "region": [\n    {\n      "replicaInfo": [\n'
  for i in $(seq 1 "${REPLICA_COUNT}"); do
    comma=','
    [[ "${i}" -eq "${REPLICA_COUNT}" ]] && comma=''
    printf '        { "id": "%s", "ip": "127.0.0.1", "port": %s }%s\n' \
      "${i}" "$((BASE_PORT + i))" "${comma}"
  done
  printf '      ]\n    }\n  ]\n}\n'
} > "${SERVER_CONFIG}"

cat > "${CLIENT_CONFIG}" <<EOF
{
  "replica_info": [
    { "id": ${TOTAL_PROCESS_COUNT}, "ip": "127.0.0.1", "port": $((BASE_PORT + TOTAL_PROCESS_COUNT)) }
  ],
  "region_id": 1
}
EOF

log "starting ${TOTAL_PROCESS_COUNT} kv_service processes"
for i in $(seq 1 "${TOTAL_PROCESS_COUNT}"); do
  "${SERVER_BIN}" "${SERVER_CONFIG}" "${CERT_ROOT}/node${i}.key.pri" \
    "${CERT_ROOT}/cert_${i}.cert" > "${WORK_ROOT}/node-${i}.log" 2>&1 &
  pid="$!"
  pids+=("${pid}")
  printf '%s\n' "${pid}" > "${WORK_ROOT}/node-${i}.pid"
  log "node-${i} pid=${pid} port=$((BASE_PORT + i))"
  sleep "${START_STAGGER_SEC}"
done

ready_started_ms="$(now_ms)"
deadline=$((SECONDS + READY_TIMEOUT_SEC))
while (( SECONDS < deadline )); do
  ready_count=0
  for i in $(seq 1 "${TOTAL_PROCESS_COUNT}"); do
    if grep -q "Server ${i} is ready" "${WORK_ROOT}/node-${i}.log" 2>/dev/null; then
      ready_count=$((ready_count + 1))
    fi
  done
  printf '%s ready=%s/%s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
    "${ready_count}" "${TOTAL_PROCESS_COUNT}" >> "${WORK_ROOT}/readiness.trace"
  [[ "${ready_count}" -eq "${TOTAL_PROCESS_COUNT}" ]] && break
  for pid in "${pids[@]}"; do
    if ! kill -0 "${pid}" >/dev/null 2>&1; then
      error_code="PROCESS_EXITED_BEFORE_READY"
      log "ERROR: process ${pid} exited before readiness"
      exit 30
    fi
  done
  sleep 1
done

ready_duration_ms=$(($(now_ms) - ready_started_ms))
if [[ "${ready_count}" -ne "${TOTAL_PROCESS_COUNT}" ]]; then
  error_code="READINESS_TIMEOUT"
  log "ERROR: readiness timeout (${ready_count}/${TOTAL_PROCESS_COUNT})"
  exit 31
fi
log "all processes ready (${ready_count}/${TOTAL_PROCESS_COUNT})"

ops_started_ms="$(now_ms)"
printf 'operation,setDurationMs,getDurationMs,status\n' > "${WORK_ROOT}/operations.csv"
for operation_index in $(seq 1 "${OPERATION_COUNT}"); do
  op_label="$(printf '%03d' "${operation_index}")"
  op_key="${TEST_KEY}-${op_label}"
  op_value="${TEST_VALUE}-${op_label}"
  set_log="${WORK_ROOT}/client-set-${op_label}.log"
  get_log="${WORK_ROOT}/client-get-${op_label}.log"

  set_started_ms="$(now_ms)"
  set +e
  timeout "${CLIENT_TIMEOUT_SEC}" "${CLIENT_BIN}" "${CLIENT_CONFIG}" set \
    "${op_key}" "${op_value}" > "${set_log}" 2>&1
  client_set_rc=$?
  set -e
  set_duration_ms=$(($(now_ms) - set_started_ms))
  if [[ "${client_set_rc}" -eq 124 ]]; then
    error_code="CLIENT_SET_TIMEOUT_OP_${op_label}"
    exit 40
  fi
  if [[ "${client_set_rc}" -ne 0 ]] || ! grep -Eq "ret = 0|client set ret = 0" "${set_log}"; then
    error_code="CLIENT_SET_FAILED_OP_${op_label}"
    exit 41
  fi

  sleep "${AFTER_SET_SLEEP_SEC}"
  get_started_ms="$(now_ms)"
  set +e
  timeout "${CLIENT_TIMEOUT_SEC}" "${CLIENT_BIN}" "${CLIENT_CONFIG}" get \
    "${op_key}" > "${get_log}" 2>&1
  client_get_rc=$?
  set -e
  get_duration_ms=$(($(now_ms) - get_started_ms))
  if [[ "${client_get_rc}" -eq 124 ]]; then
    error_code="CLIENT_GET_TIMEOUT_OP_${op_label}"
    exit 42
  fi
  if [[ "${client_get_rc}" -ne 0 ]] || ! grep -Fq "${op_value}" "${get_log}"; then
    error_code="CLIENT_GET_FAILED_OP_${op_label}"
    exit 43
  fi

  printf '%s,%s,%s,PASSED\n' "${op_label}" "${set_duration_ms}" "${get_duration_ms}" >> "${WORK_ROOT}/operations.csv"
  passed_operations="${operation_index}"
done
operations_duration_ms=$(($(now_ms) - ops_started_ms))
total_duration_ms=$(($(now_ms) - start_ms))
result_status="RUNTIME_WARM_CLUSTER_PASSED"
error_code=""
log "runtime warm-cluster passed operations=${passed_operations}/${OPERATION_COUNT}"
PAYLOAD
}

run_protocol() {
  local protocol="$1"
  local index="$2"
  local image bin_root base_port container payload protocol_dir image_id size_bytes started ended duration status notes exit_code
  image="$(protocol_image "${protocol}")"
  bin_root="$(protocol_bin_root "${protocol}")"
  base_port=$((BASE_PORT_START + index * PORT_STRIDE))
  container="$(container_name_for "${protocol}")"
  protocol_dir="${LOG_ROOT}/${protocol}"
  payload="${LOG_ROOT}/payload-${protocol}.sh"
  mkdir -p "${protocol_dir}"

  if ! docker image inspect "${image}" >/dev/null 2>&1; then
    printf '%s,%s,%s,%s,FAILED,0,,,,%s,,,%s\n' "${VERSION}" "${RUN_ID}" "${protocol}" "${image}" "${OPERATION_COUNT}" "IMAGE_NOT_FOUND" >> "${SUMMARY_CSV}"
    return 1
  fi
  image_id="$(docker image inspect "${image}" --format '{{.Id}}')"
  size_bytes="$(docker image inspect "${image}" --format '{{.Size}}')"

  write_payload "${payload}"
  docker rm -f "${container}" >/dev/null 2>&1 || true
  started="$(now_ms)"
  docker create --name "${container}" --entrypoint sleep "${image}" infinity > "${protocol_dir}/container.id"
  docker cp "${payload}" "${container}:/tmp/chatay-runtime-image-kv-cluster.sh"
  docker start "${container}" >/dev/null

  set +e
  docker exec \
    -e "PROTOCOL=${protocol}" -e "BIN_ROOT=${bin_root}" -e "BASE_PORT=${base_port}" \
    -e "OPERATION_COUNT=${OPERATION_COUNT}" -e "READY_TIMEOUT_SEC=${READY_TIMEOUT_SEC}" \
    -e "CLIENT_TIMEOUT_SEC=${CLIENT_TIMEOUT_SEC}" -e "START_STAGGER_SEC=${START_STAGGER_SEC}" \
    -e "AFTER_SET_SLEEP_SEC=${AFTER_SET_SLEEP_SEC}" -e "RUN_ID=${RUN_ID}" \
    "${container}" bash /tmp/chatay-runtime-image-kv-cluster.sh > "${protocol_dir}/docker-exec.log" 2>&1
  exit_code=$?
  set -e

  docker cp "${container}:/work/." "${protocol_dir}" >/dev/null 2>&1 || true
  docker rm -f "${container}" >/dev/null 2>&1 || true
  ended="$(now_ms)"
  duration=$((ended - started))

  status="PASSED"
  notes="runtime warm-cluster passed"
  [[ "${exit_code}" -ne 0 ]] && status="FAILED" && notes="runtime warm-cluster failed rc=${exit_code}"

  local ready_duration operations_duration passed_operations error_code
  ready_duration=""
  operations_duration=""
  passed_operations=""
  error_code=""
  if [[ -f "${protocol_dir}/manifest.json" ]]; then
    ready_duration="$(python3 -c 'import json,sys; d=json.load(open(sys.argv[1])); print(d.get("readyDurationMs",""))' "${protocol_dir}/manifest.json")"
    operations_duration="$(python3 -c 'import json,sys; d=json.load(open(sys.argv[1])); print(d.get("operationsDurationMs",""))' "${protocol_dir}/manifest.json")"
    passed_operations="$(python3 -c 'import json,sys; d=json.load(open(sys.argv[1])); print(d.get("passedOperations",""))' "${protocol_dir}/manifest.json")"
    error_code="$(python3 -c 'import json,sys; d=json.load(open(sys.argv[1])); print(d.get("errorCode",""))' "${protocol_dir}/manifest.json")"
  fi

  printf '%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n' \
    "${VERSION}" "${RUN_ID}" "${protocol}" "${image}" "${status}" "${duration}" \
    "${image_id}" "${size_bytes}" "${base_port}" "${OPERATION_COUNT}" \
    "${passed_operations}" "${ready_duration}" "${operations_duration}" "${error_code:-${notes}}" >> "${SUMMARY_CSV}"

  [[ "${exit_code}" -eq 0 ]]
}

write_summary_md() {
  {
    printf '# Runtime Image Warm-Cluster Summary - %s\n\n' "${VERSION}"
    printf 'Run ID: `%s`\n\n' "${RUN_ID}"
    printf '| Protocol | Image | Status | Operations | Passed | Ready ms | Ops ms | Notes |\n'
    printf '| --- | --- | --- | ---: | ---: | ---: | ---: | --- |\n'
    tail -n +2 "${SUMMARY_CSV}" | while IFS=',' read -r semver run_id protocol image status duration image_id size base_port operations passed ready ops notes; do
      printf '| `%s` | `%s` | `%s` | %s | %s | %s | %s | `%s` |\n' \
        "${protocol}" "${image}" "${status}" "${operations}" "${passed}" "${ready}" "${ops}" "${notes}"
    done
    printf '\n## Claim Boundary\n\n'
    printf 'This run executes frozen runtime images only. It validates local warm-cluster behavior and captures timing slices, but it is not yet a heterogeneous-network benchmark.\n'
  } > "${SUMMARY_MD}"
}

printf 'semver,runId,protocol,image,status,durationMs,imageId,sizeBytes,basePort,operationCount,passedOperations,readyDurationMs,operationsDurationMs,notes\n' > "${SUMMARY_CSV}"

overall_status=0
IFS=',' read -r -a protocol_list <<< "${PROTOCOLS}"
index=0
for protocol in "${protocol_list[@]}"; do
  protocol="$(echo "${protocol}" | tr -d '[:space:]')"
  [[ -z "${protocol}" ]] && continue
  if ! run_protocol "${protocol}" "${index}"; then
    overall_status=1
  fi
  index=$((index + 1))
done

write_summary_md
printf 'summary=%s\nsummary_md=%s\n' "${SUMMARY_CSV}" "${SUMMARY_MD}"
exit "${overall_status}"
