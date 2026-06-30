#!/usr/bin/env bash

set -Eeuo pipefail

VERSION="${CHATAY_SEMVER:-v2.17.4-alpha.1}"
ROOT="$(git rev-parse --show-toplevel)"
MATRIX_RUN_ID="${RUN_ID:-$(date -u +%Y%m%dT%H%M%SZ)-hs2-fault-matrix}"
LOG_ROOT="${CHATAY_LOG_ROOT:-${ROOT}/documents/chatay-porting/${VERSION}/logs/${MATRIX_RUN_ID}}"
SUMMARY_CSV="${LOG_ROOT}/summary.csv"
BASE_PORT_START="${HS2_FAULT_BASE_PORT:-26400}"
REQUIRED_SCENARIOS="${HS2_FAULT_REQUIRED_SCENARIOS:-baseline,stopped_nonleader,slow_start}"

mkdir -p "${LOG_ROOT}"
if [[ ! -f "${LOG_ROOT}/../.gitignore" ]]; then
  {
    printf '*\n'
    printf '!.gitignore\n'
  } > "${LOG_ROOT}/../.gitignore"
fi

contains_required() {
  local scenario="$1"
  [[ ",${REQUIRED_SCENARIOS}," == *",${scenario},"* ]]
}

json_field() {
  local file="$1"
  local field="$2"
  if [[ ! -f "${file}" ]]; then
    printf ''
    return
  fi
  sed -n "s/.*\"${field}\": \"\\([^\"]*\\)\".*/\\1/p" "${file}" | tail -n 1
}

run_scenario() {
  local scenario="$1"
  local base_port="$2"
  shift 2
  local run_id="${MATRIX_RUN_ID}-${scenario}"
  local scenario_log="${LOG_ROOT}/${scenario}.stdout.log"
  local rc=0

  printf '[%s] scenario=%s base_port=%s\n' \
    "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "${scenario}" "${base_port}" |
    tee -a "${LOG_ROOT}/matrix.log"

  set +e
  CHATAY_SEMVER="${VERSION}" \
    RUN_ID="${run_id}" \
    HS2_BASE_PORT="${base_port}" \
    HS2_OPERATION_COUNT="${HS2_OPERATION_COUNT:-1}" \
    HS2_READY_TIMEOUT_SEC="${HS2_READY_TIMEOUT_SEC:-90}" \
    HS2_CLIENT_TIMEOUT_SEC="${HS2_CLIENT_TIMEOUT_SEC:-45}" \
    BAZEL_JOBS="${BAZEL_JOBS:-4}" \
    "$@" "${ROOT}/tools/chatay/hs2/run_hs2_kv_smoke_cluster.sh" \
    > "${scenario_log}" 2>&1
  rc=$?
  set -e

  local manifest="${ROOT}/documents/chatay-porting/${VERSION}/logs/${run_id}/manifest.json"
  local status error_code expectation
  status="$(json_field "${manifest}" "status")"
  error_code="$(json_field "${manifest}" "errorCode")"
  expectation="classified"
  if contains_required "${scenario}"; then
    expectation="required-pass"
  fi

  printf '%s,%s,%s,%s,%s,%s,%s,%s\n' \
    "${scenario}" "${run_id}" "${base_port}" "${rc}" "${status}" \
    "${error_code}" "${expectation}" "${scenario_log}" >> "${SUMMARY_CSV}"

  if [[ "${expectation}" == "required-pass" && "${status}" != "RUNTIME_SMOKE_PASSED" ]]; then
    printf '[%s] required scenario failed scenario=%s status=%s error=%s\n' \
      "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "${scenario}" "${status}" "${error_code}" |
      tee -a "${LOG_ROOT}/matrix.log"
    return 1
  fi

  printf '[%s] scenario=%s status=%s error=%s expectation=%s\n' \
    "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "${scenario}" "${status}" \
    "${error_code}" "${expectation}" | tee -a "${LOG_ROOT}/matrix.log"
  return 0
}

printf 'scenario,runId,basePort,exitCode,status,errorCode,expectation,stdoutLog\n' \
  > "${SUMMARY_CSV}"

run_scenario baseline "$((BASE_PORT_START + 0))" env
run_scenario stopped_nonleader "$((BASE_PORT_START + 100))" \
  env HS2_STOP_NODE_AFTER_READY=3
run_scenario slow_start "$((BASE_PORT_START + 200))" \
  env HS2_SLOW_NODE_ID=4 HS2_SLOW_NODE_DELAY_SEC="${HS2_SLOW_NODE_DELAY_SEC:-5}"
run_scenario stopped_leader "$((BASE_PORT_START + 300))" \
  env HS2_STOP_NODE_AFTER_READY=1 || true

printf '[%s] fault matrix complete summary=%s\n' \
  "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "${SUMMARY_CSV}" | tee -a "${LOG_ROOT}/matrix.log"
