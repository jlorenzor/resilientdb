#!/usr/bin/env bash

set -Eeuo pipefail

VERSION="${CHATAY_SEMVER:-v2.18.1-alpha.1}"
PBFT_IMAGE="${CHATAY_PBFT_IMAGE:-chatay-resilientdb-pbft:runtime-v2.13.20-alpha.1}"
HS1_IMAGE="${CHATAY_HS1_IMAGE:-chatay-resilientdb-hs1-pr100:v2.14.10-beta.1}"
HS2_IMAGE="${CHATAY_HS2_IMAGE:-chatay-resilientdb-hs2:v2.18.0-alpha.1}"

ROOT="$(git rev-parse --show-toplevel)"
VERSION_DIR="${ROOT}/documents/chatay-porting/${VERSION}"
JSON_OUT="${VERSION_DIR}/protocol-image-inventory.json"
MD_OUT="${VERSION_DIR}/protocol-image-inventory.md"

mkdir -p "${VERSION_DIR}"

images=(
  "pbft|PBFT baseline|${PBFT_IMAGE}|ResilientDB PBFT runtime baseline"
  "hs1-pr100|HS1/PR100 bridge|${HS1_IMAGE}|HotStuff-like PR100 runtime bridge"
  "hs2|HS2 experimental|${HS2_IMAGE}|Post-hardening HS2 experimental runtime"
)

inspect_field() {
  local image="$1"
  local field="$2"
  docker image inspect "${image}" --format "${field}"
}

require_image() {
  local image="$1"
  if ! docker image inspect "${image}" >/dev/null 2>&1; then
    printf 'missing required image: %s\n' "${image}" >&2
    return 1
  fi
}

for spec in "${images[@]}"; do
  IFS='|' read -r _protocol _role image _notes <<<"${spec}"
  require_image "${image}"
done

{
  printf '{\n'
  printf '  "schemaVersion": "chatay-protocol-image-inventory-v1",\n'
  printf '  "semver": "%s",\n' "${VERSION}"
  printf '  "sourceCommit": "%s",\n' "$(git -C "${ROOT}" rev-parse HEAD)"
  printf '  "sourceBranch": "%s",\n' "$(git -C "${ROOT}" rev-parse --abbrev-ref HEAD)"
  printf '  "images": [\n'

  first=1
  for spec in "${images[@]}"; do
    IFS='|' read -r protocol role image notes <<<"${spec}"
    image_id="$(inspect_field "${image}" '{{.Id}}')"
    image_size="$(inspect_field "${image}" '{{.Size}}')"
    image_created="$(inspect_field "${image}" '{{.Created}}')"
    image_labels="$(inspect_field "${image}" '{{json .Config.Labels}}')"

    if [[ "${first}" -eq 0 ]]; then
      printf ',\n'
    fi
    first=0

    printf '    {\n'
    printf '      "protocol": "%s",\n' "${protocol}"
    printf '      "role": "%s",\n' "${role}"
    printf '      "image": "%s",\n' "${image}"
    printf '      "imageId": "%s",\n' "${image_id}"
    printf '      "sizeBytes": %s,\n' "${image_size}"
    printf '      "created": "%s",\n' "${image_created}"
    printf '      "notes": "%s",\n' "${notes}"
    printf '      "labels": %s\n' "${image_labels}"
    printf '    }'
  done

  printf '\n  ]\n'
  printf '}\n'
} > "${JSON_OUT}"

{
  printf '# Protocol Image Inventory - %s\n\n' "${VERSION}"
  printf 'This inventory locks the local Docker images used for the next comparable PBFT vs HS1 vs HS2 run.\n\n'
  printf '| Protocol | Role | Image | Image ID | Size | Notes |\n'
  printf '| --- | --- | --- | --- | ---: | --- |\n'

  for spec in "${images[@]}"; do
    IFS='|' read -r protocol role image notes <<<"${spec}"
    image_id="$(inspect_field "${image}" '{{.Id}}')"
    image_size="$(inspect_field "${image}" '{{.Size}}')"
    short_id="${image_id#sha256:}"
    short_id="${short_id:0:12}"
    size_mb=$(( (image_size + 500000) / 1000000 ))
    printf '| `%s` | %s | `%s` | `%s` | %s MB | %s |\n' \
      "${protocol}" "${role}" "${image}" "sha256:${short_id}" "${size_mb}" "${notes}"
  done

  printf '\n## Claim Boundary\n\n'
  printf 'This file proves local image identity and approximate size. It does not prove comparable runtime behavior by itself.\n'
  printf 'The segmented runner must still measure build time, cold-start time, warm-cluster operation time and phase traces separately.\n'
} > "${MD_OUT}"

printf 'wrote %s\n' "${JSON_OUT}"
printf 'wrote %s\n' "${MD_OUT}"
