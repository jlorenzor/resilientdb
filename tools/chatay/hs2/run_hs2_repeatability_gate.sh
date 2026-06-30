#!/usr/bin/env bash

set -Eeuo pipefail

VERSION="${CHATAY_SEMVER:-v2.17.5-beta.1}"
ROOT="$(git rev-parse --show-toplevel)"
GATE_RUN_ID="${RUN_ID:-$(date -u +%Y%m%dT%H%M%SZ)-hs2-repeatability}"
LOG_ROOT="${CHATAY_LOG_ROOT:-${ROOT}/documents/chatay-porting/${VERSION}/logs/${GATE_RUN_ID}}"
SUMMARY_CSV="${LOG_ROOT}/summary.csv"
BASE_PORT_START="${HS2_REPEAT_BASE_PORT:-26800}"
OPERATION_COUNTS="${HS2_REPEAT_OPERATION_COUNTS:-30,100}"
AFTER_SET_SLEEP_SEC="${HS2_AFTER_SET_SLEEP_SEC:-0.2}"

mkdir -p "${LOG_ROOT}"
if [[ ! -f "${LOG_ROOT}/../.gitignore" ]]; then
  {
    printf '*\n'
    printf '!.gitignore\n'
  } > "${LOG_ROOT}/../.gitignore"
fi

json_field() {
  local file="$1"
  local field="$2"
  if [[ ! -f "${file}" ]]; then
    printf ''
    return
  fi
  sed -n "s/.*\"${field}\": \"\\([^\"]*\\)\".*/\\1/p" "${file}" | tail -n 1
}

json_number() {
  local file="$1"
  local field="$2"
  if [[ ! -f "${file}" ]]; then
    printf ''
    return
  fi
  sed -n "s/.*\"${field}\": \\([0-9][0-9]*\\).*/\\1/p" "${file}" | tail -n 1
}

printf 'operationCount,runId,basePort,exitCode,status,errorCode,passedOperations,stdoutLog\n' \
  > "${SUMMARY_CSV}"

IFS=',' read -r -a counts <<< "${OPERATION_COUNTS}"
index=0
for count in "${counts[@]}"; do
  count="$(echo "${count}" | tr -d '[:space:]')"
  if [[ -z "${count}" ]]; then
    continue
  fi
  base_port=$((BASE_PORT_START + index * 100))
  run_id="${GATE_RUN_ID}-${count}"
  stdout_log="${LOG_ROOT}/${count}.stdout.log"
  rc=0

  printf '[%s] repeatability count=%s base_port=%s\n' \
    "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "${count}" "${base_port}" |
    tee -a "${LOG_ROOT}/repeatability.log"

  set +e
  CHATAY_SEMVER="${VERSION}" \
    RUN_ID="${run_id}" \
    HS2_BASE_PORT="${base_port}" \
    HS2_OPERATION_COUNT="${count}" \
    HS2_AFTER_SET_SLEEP_SEC="${AFTER_SET_SLEEP_SEC}" \
    HS2_READY_TIMEOUT_SEC="${HS2_READY_TIMEOUT_SEC:-90}" \
    HS2_CLIENT_TIMEOUT_SEC="${HS2_CLIENT_TIMEOUT_SEC:-45}" \
    BAZEL_JOBS="${BAZEL_JOBS:-4}" \
    "${ROOT}/tools/chatay/hs2/run_hs2_kv_smoke_cluster.sh" \
    > "${stdout_log}" 2>&1
  rc=$?
  set -e

  manifest="${ROOT}/documents/chatay-porting/${VERSION}/logs/${run_id}/manifest.json"
  status="$(json_field "${manifest}" "status")"
  error_code="$(json_field "${manifest}" "errorCode")"
  passed_operations="$(json_number "${manifest}" "passedOperations")"

  printf '%s,%s,%s,%s,%s,%s,%s,%s\n' \
    "${count}" "${run_id}" "${base_port}" "${rc}" "${status}" \
    "${error_code}" "${passed_operations}" "${stdout_log}" >> "${SUMMARY_CSV}"

  if [[ "${status}" != "RUNTIME_SMOKE_PASSED" ||
        "${passed_operations}" != "${count}" ]]; then
    printf '[%s] repeatability failed count=%s status=%s passed=%s error=%s\n' \
      "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "${count}" "${status}" \
      "${passed_operations}" "${error_code}" | tee -a "${LOG_ROOT}/repeatability.log"
    exit 1
  fi

  printf '[%s] repeatability passed count=%s passed=%s/%s\n' \
    "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "${count}" "${passed_operations}" \
    "${count}" | tee -a "${LOG_ROOT}/repeatability.log"
  index=$((index + 1))
done

printf '[%s] repeatability gate complete summary=%s\n' \
  "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "${SUMMARY_CSV}" |
  tee -a "${LOG_ROOT}/repeatability.log"
