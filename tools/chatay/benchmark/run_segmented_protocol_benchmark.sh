#!/usr/bin/env bash

set -Eeuo pipefail

VERSION="${CHATAY_SEMVER:-v2.18.2-alpha.1}"
RUN_ID="${RUN_ID:-$(date -u +%Y%m%dT%H%M%SZ)-segmented-runner}"
ROOT="$(git rev-parse --show-toplevel)"
VERSION_DIR="${ROOT}/documents/chatay-porting/${VERSION}"
LOG_ROOT="${CHATAY_LOG_ROOT:-${VERSION_DIR}/logs/${RUN_ID}}"
SUMMARY_CSV="${LOG_ROOT}/segmented-summary.csv"
MANIFEST_JSON="${LOG_ROOT}/segmented-manifest.json"
PLAN_MD="${VERSION_DIR}/segmented-runner-plan.md"

PROTOCOLS="${CHATAY_PROTOCOLS:-pbft,hs1-pr100,hs2}"
SEGMENTS="${CHATAY_SEGMENTS:-image-check,build,cold-start,warm-cluster,phase-trace}"
EXECUTE_SEGMENTS="${CHATAY_EXECUTE_SEGMENTS:-image-check}"

PBFT_IMAGE="${CHATAY_PBFT_IMAGE:-chatay-resilientdb-pbft:runtime-v2.13.20-alpha.1}"
HS1_IMAGE="${CHATAY_HS1_IMAGE:-chatay-resilientdb-hs1-pr100:v2.14.10-beta.1}"
HS2_IMAGE="${CHATAY_HS2_IMAGE:-chatay-resilientdb-hs2:v2.18.0-alpha.1}"

mkdir -p "${LOG_ROOT}" "${VERSION_DIR}/logs"
if [[ ! -f "${VERSION_DIR}/logs/.gitignore" ]]; then
  {
    printf '*\n'
    printf '!.gitignore\n'
  } > "${VERSION_DIR}/logs/.gitignore"
fi

now_ms() {
  date -u +%s%3N
}

contains_csv() {
  local csv="$1"
  local needle="$2"
  local item
  IFS=',' read -r -a parts <<< "${csv}"
  for item in "${parts[@]}"; do
    item="$(echo "${item}" | tr -d '[:space:]')"
    if [[ "${item}" == "${needle}" ]]; then
      return 0
    fi
  done
  return 1
}

protocol_image() {
  case "$1" in
    pbft) printf '%s' "${PBFT_IMAGE}" ;;
    hs1-pr100) printf '%s' "${HS1_IMAGE}" ;;
    hs2) printf '%s' "${HS2_IMAGE}" ;;
    *) printf 'unknown protocol: %s\n' "$1" >&2; return 2 ;;
  esac
}

record_segment() {
  local protocol="$1"
  local segment="$2"
  local status="$3"
  local duration_ms="$4"
  local image="$5"
  local image_id="$6"
  local size_bytes="$7"
  local command="$8"
  local notes="$9"

  printf '%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n' \
    "${VERSION}" "${RUN_ID}" "${protocol}" "${segment}" "${status}" \
    "${duration_ms}" "${image}" "${image_id}" "${size_bytes}" "${command}" \
    "${notes}" >> "${SUMMARY_CSV}"
}

run_image_check() {
  local protocol="$1"
  local image="$2"
  local started ended duration image_id size_bytes
  started="$(now_ms)"
  if docker image inspect "${image}" >/dev/null 2>&1; then
    image_id="$(docker image inspect "${image}" --format '{{.Id}}')"
    size_bytes="$(docker image inspect "${image}" --format '{{.Size}}')"
    ended="$(now_ms)"
    duration=$((ended - started))
    record_segment "${protocol}" image-check PASSED "${duration}" "${image}" \
      "${image_id}" "${size_bytes}" "docker image inspect" "image available"
    return 0
  fi
  ended="$(now_ms)"
  duration=$((ended - started))
  record_segment "${protocol}" image-check FAILED "${duration}" "${image}" \
    "" "" "docker image inspect" "image missing"
  return 1
}

write_plan() {
  {
    printf '# Segmented Benchmark Runner Plan - %s\n\n' "${VERSION}"
    printf 'Run ID: `%s`\n\n' "${RUN_ID}"
    printf '## Protocols\n\n'
    printf '| Protocol | Runtime image |\n'
    printf '| --- | --- |\n'
    printf '| PBFT | `%s` |\n' "${PBFT_IMAGE}"
    printf '| HS1/PR100 | `%s` |\n' "${HS1_IMAGE}"
    printf '| HS2 | `%s` |\n\n' "${HS2_IMAGE}"
    printf '## Segments\n\n'
    printf '| Segment | v2.18.2 behavior | Later gate |\n'
    printf '| --- | --- | --- |\n'
    printf '| `image-check` | Executed now with Docker inspect. | Input gate for all benchmark runs. |\n'
    printf '| `build` | Planned and represented in the schema. | Real timing in a later build/runtime gate. |\n'
    printf '| `cold-start` | Planned and represented in the schema. | Real timing in `v2.18.4-beta.1`. |\n'
    printf '| `warm-cluster` | Planned and represented in the schema. | Real timing in `v2.18.3-beta.1`. |\n'
    printf '| `phase-trace` | Planned and represented in the schema. | Real traces in `v2.18.5-rc.1`. |\n\n'
    printf '## Claim Boundary\n\n'
    printf 'This alpha runner validates image availability and creates a common output schema. It does not yet produce comparative performance claims.\n'
  } > "${PLAN_MD}"
}

write_manifest() {
  local branch commit finished_at
  branch="$(git -C "${ROOT}" rev-parse --abbrev-ref HEAD 2>/dev/null || true)"
  commit="$(git -C "${ROOT}" rev-parse HEAD 2>/dev/null || true)"
  finished_at="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
  cat > "${MANIFEST_JSON}" <<EOF
{
  "schemaVersion": "chatay-segmented-runner-manifest-v1",
  "semver": "${VERSION}",
  "runId": "${RUN_ID}",
  "branch": "${branch}",
  "commit": "${commit}",
  "protocols": "${PROTOCOLS}",
  "segments": "${SEGMENTS}",
  "executedSegments": "${EXECUTE_SEGMENTS}",
  "summaryCsv": "${SUMMARY_CSV}",
  "plan": "${PLAN_MD}",
  "finishedAt": "${finished_at}"
}
EOF
}

printf 'semver,runId,protocol,segment,status,durationMs,image,imageId,sizeBytes,command,notes\n' > "${SUMMARY_CSV}"

write_plan

overall_status=0
IFS=',' read -r -a protocol_list <<< "${PROTOCOLS}"
IFS=',' read -r -a segment_list <<< "${SEGMENTS}"

for protocol in "${protocol_list[@]}"; do
  protocol="$(echo "${protocol}" | tr -d '[:space:]')"
  [[ -z "${protocol}" ]] && continue
  image="$(protocol_image "${protocol}")"

  for segment in "${segment_list[@]}"; do
    segment="$(echo "${segment}" | tr -d '[:space:]')"
    [[ -z "${segment}" ]] && continue

    if contains_csv "${EXECUTE_SEGMENTS}" "${segment}"; then
      case "${segment}" in
        image-check)
          run_image_check "${protocol}" "${image}" || overall_status=1
          ;;
        *)
          record_segment "${protocol}" "${segment}" UNSUPPORTED 0 "${image}" "" "" \
            "not implemented" "segment declared but not executable in v2.18.2"
          overall_status=1
          ;;
      esac
    else
      record_segment "${protocol}" "${segment}" PLANNED 0 "${image}" "" "" \
        "not executed" "schema placeholder for later gate"
    fi
  done
done

write_manifest

printf 'summary=%s\n' "${SUMMARY_CSV}"
printf 'manifest=%s\n' "${MANIFEST_JSON}"
printf 'plan=%s\n' "${PLAN_MD}"

exit "${overall_status}"
