#!/usr/bin/env bash

set -Eeuo pipefail

VERSION="${CHATAY_SEMVER:-v2.18.4-beta.1}"
RUN_ID="${RUN_ID:-$(date -u +%Y%m%dT%H%M%SZ)-runtime-coldstart}"
ROOT="$(git rev-parse --show-toplevel)"
VERSION_DIR="${ROOT}/documents/chatay-porting/${VERSION}"
LOG_ROOT="${CHATAY_LOG_ROOT:-${VERSION_DIR}/logs/${RUN_ID}}"
SUMMARY_CSV="${LOG_ROOT}/cold-start-summary.csv"
SUMMARY_MD="${LOG_ROOT}/cold-start-summary.md"
WARM_RUNNER="${ROOT}/tools/chatay/benchmark/run_runtime_image_kv_cluster.sh"

PROTOCOLS="${CHATAY_PROTOCOLS:-pbft,hs1-pr100,hs2}"
REPEAT_COUNT="${CHATAY_REPEAT_COUNT:-3}"
# Keep benchmark service ports below the usual Linux ephemeral range
# (32768-60999) to avoid transient bind conflicts during fast local startup.
BASE_PORT_START="${CHATAY_BASE_PORT_START:-25000}"
PORT_STRIDE="${CHATAY_PORT_STRIDE:-100}"

mkdir -p "${LOG_ROOT}" "${VERSION_DIR}/logs"
if [[ ! -f "${VERSION_DIR}/logs/.gitignore" ]]; then
  printf '*\n!.gitignore\n' > "${VERSION_DIR}/logs/.gitignore"
fi

if [[ ! -f "${WARM_RUNNER}" ]]; then
  printf 'missing warm runner: %s\n' "${WARM_RUNNER}" >&2
  exit 2
fi

csv_value() {
  local file="$1"
  local column="$2"
  python3 - "$file" "$column" <<'PY'
import csv
import sys

path, column = sys.argv[1], sys.argv[2]
with open(path, newline="", encoding="utf-8") as fh:
    reader = csv.DictReader(fh)
    row = next(reader)
    print(row.get(column, ""))
PY
}

append_child_result() {
  local protocol="$1"
  local repeat="$2"
  local child_summary="$3"
  local status duration image image_id size base_port operations passed ready ops notes

  status="$(csv_value "${child_summary}" status)"
  duration="$(csv_value "${child_summary}" durationMs)"
  image="$(csv_value "${child_summary}" image)"
  image_id="$(csv_value "${child_summary}" imageId)"
  size="$(csv_value "${child_summary}" sizeBytes)"
  base_port="$(csv_value "${child_summary}" basePort)"
  operations="$(csv_value "${child_summary}" operationCount)"
  passed="$(csv_value "${child_summary}" passedOperations)"
  ready="$(csv_value "${child_summary}" readyDurationMs)"
  ops="$(csv_value "${child_summary}" operationsDurationMs)"
  notes="$(csv_value "${child_summary}" notes)"

  printf '%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n' \
    "${VERSION}" "${RUN_ID}" "${protocol}" "${repeat}" "${image}" "${status}" \
    "${duration}" "${ready}" "${ops}" "${image_id}" "${size}" "${base_port}" \
    "${operations}" "${passed}" "${notes}" >> "${SUMMARY_CSV}"
}

write_summary_md() {
  {
    printf '# Runtime Image Cold-Start Matrix - %s\n\n' "${VERSION}"
    printf 'Run ID: `%s`\n\n' "${RUN_ID}"
    printf '| Protocol | Repeat | Status | Container total ms | Process ready ms | Ops ms | Passed |\n'
    printf '| --- | ---: | --- | ---: | ---: | ---: | ---: |\n'
    tail -n +2 "${SUMMARY_CSV}" | while IFS=',' read -r semver run_id protocol repeat image status duration ready ops image_id size base_port operations passed notes; do
      printf '| `%s` | %s | `%s` | %s | %s | %s | %s/%s |\n' \
        "${protocol}" "${repeat}" "${status}" "${duration}" "${ready}" "${ops}" "${passed}" "${operations}"
    done
    printf '\n## Interpretation Boundary\n\n'
    printf 'Container total time includes Docker create/start/exec/copy/remove overhead. Process ready time is measured inside the container after KV processes start. Operations time covers one SET/GET pair.\n'
  } > "${SUMMARY_MD}"
}

printf 'semver,runId,protocol,repeat,image,status,containerTotalDurationMs,processReadyDurationMs,operationsDurationMs,imageId,sizeBytes,basePort,operationCount,passedOperations,notes\n' > "${SUMMARY_CSV}"

overall_status=0
IFS=',' read -r -a protocol_list <<< "${PROTOCOLS}"
protocol_index=0
for protocol in "${protocol_list[@]}"; do
  protocol="$(echo "${protocol}" | tr -d '[:space:]')"
  [[ -z "${protocol}" ]] && continue

  for repeat in $(seq 1 "${REPEAT_COUNT}"); do
    child_id="${RUN_ID}-${protocol}-r$(printf '%02d' "${repeat}")"
    child_log="${LOG_ROOT}/${protocol}/repeat-$(printf '%02d' "${repeat}")"
    child_port=$((BASE_PORT_START + protocol_index * PORT_STRIDE + repeat * 10))
    mkdir -p "${child_log}"

    set +e
    CHATAY_SEMVER="${VERSION}" \
    RUN_ID="${child_id}" \
    CHATAY_LOG_ROOT="${child_log}" \
    CHATAY_PROTOCOLS="${protocol}" \
    CHATAY_OPERATION_COUNT=1 \
    CHATAY_BASE_PORT_START="${child_port}" \
    bash "${WARM_RUNNER}" > "${child_log}/runner.log" 2>&1
    child_rc=$?
    set -e

    child_summary="${child_log}/runtime-warm-summary.csv"
    if [[ -f "${child_summary}" ]]; then
      append_child_result "${protocol}" "${repeat}" "${child_summary}"
    else
      printf '%s,%s,%s,%s,,FAILED,0,0,0,,,0,1,0,CHILD_SUMMARY_MISSING\n' \
        "${VERSION}" "${RUN_ID}" "${protocol}" "${repeat}" >> "${SUMMARY_CSV}"
      child_rc=1
    fi

    if [[ "${child_rc}" -ne 0 ]]; then
      overall_status=1
    fi
  done

  protocol_index=$((protocol_index + 1))
done

write_summary_md
printf 'summary=%s\nsummary_md=%s\n' "${SUMMARY_CSV}" "${SUMMARY_MD}"
exit "${overall_status}"
