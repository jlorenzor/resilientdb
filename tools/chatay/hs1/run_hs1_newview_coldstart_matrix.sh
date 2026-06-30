#!/usr/bin/env bash

set -Eeuo pipefail

VERSION="${CHATAY_SEMVER:-v2.14.9-beta.1}"
REPEAT_COUNT="${HS1_MATRIX_REPEAT_COUNT:-3}"
CLIENT_TIMEOUT_SEC="${HS1_CLIENT_TIMEOUT_SEC:-30}"
BASE_PORT="${HS1_BASE_PORT:-22000}"
ROOT="$(git rev-parse --show-toplevel)"
LOG_PARENT="${ROOT}/documents/chatay-porting/${VERSION}/logs"
SUMMARY_CSV="${LOG_PARENT}/summary.csv"

mkdir -p "${LOG_PARENT}"
if [[ ! -f "${LOG_PARENT}/.gitignore" ]]; then
  {
    printf '*\n'
    printf '!.gitignore\n'
  } > "${LOG_PARENT}/.gitignore"
fi

printf 'mode,iteration,runId,status,errorCode,operationCount,passedOperations,basePort,clientTimeoutSec,commit,finishedAt\n' > "${SUMMARY_CSV}"

run_mode() {
  local mode="$1"
  local iteration="$2"
  local run_id
  run_id="$(date -u +%Y%m%dT%H%M%SZ)-${mode}-${iteration}"

  local -a env_args=(
    "CHATAY_SEMVER=${VERSION}"
    "RUN_ID=${run_id}"
    "HS1_CLIENT_TIMEOUT_SEC=${CLIENT_TIMEOUT_SEC}"
    "HS1_OPERATION_COUNT=1"
    "HS1_BASE_PORT=${BASE_PORT}"
  )

  if [[ "${mode}" == "reduced" ]]; then
    env_args+=(
      "CHATAY_HS1_NEWVIEW_BOOTSTRAP_BASE_SEC=0"
      "CHATAY_HS1_NEWVIEW_BOOTSTRAP_STAGGER_BY_ID=0"
      "CHATAY_HS1_NEWVIEW_ATTEMPTS=1"
      "CHATAY_HS1_NEWVIEW_RETRY_SLEEP_SEC=0"
    )
  fi

  printf '[%s] mode=%s iteration=%s runId=%s\n' \
    "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "${mode}" "${iteration}" "${run_id}"

  env "${env_args[@]}" bash "${ROOT}/tools/chatay/hs1/run_hs1_kv_warm_cluster.sh" \
    > "${LOG_PARENT}/${run_id}.stdout.log" 2>&1

  local manifest="${LOG_PARENT}/${run_id}/manifest.json"
  python3 - "${manifest}" "${mode}" "${iteration}" "${SUMMARY_CSV}" <<'PY'
import csv
import json
import sys

manifest_path, mode, iteration, summary_path = sys.argv[1:5]
with open(manifest_path, "r", encoding="utf-8") as fh:
    data = json.load(fh)

row = {
    "mode": mode,
    "iteration": iteration,
    "runId": data.get("runId", ""),
    "status": data.get("status", ""),
    "errorCode": data.get("errorCode", ""),
    "operationCount": data.get("operationCount", ""),
    "passedOperations": data.get("passedOperations", ""),
    "basePort": data.get("basePort", ""),
    "clientTimeoutSec": data.get("clientTimeoutSec", ""),
    "commit": data.get("commit", ""),
    "finishedAt": data.get("finishedAt", ""),
}

with open(summary_path, "a", encoding="utf-8", newline="") as fh:
    writer = csv.DictWriter(fh, fieldnames=list(row.keys()))
    writer.writerow(row)
PY
}

for iteration in $(seq 1 "${REPEAT_COUNT}"); do
  run_mode default "${iteration}"
  run_mode reduced "${iteration}"
done

printf '[%s] wrote %s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "${SUMMARY_CSV}"
