#!/usr/bin/env bash

set -Eeuo pipefail

VERSION="${CHATAY_SEMVER:-v2.18.3-alpha.1}"
IMAGE="${CHATAY_PBFT_RUNTIME_IMAGE:-chatay-resilientdb-pbft:${VERSION}}"
BASE_IMAGE="${CHATAY_PBFT_RUNTIME_BASE_IMAGE:-chatay-resilientdb-pbft:runtime-v2.13.20-alpha.1}"
TOOLCHAIN_IMAGE="${CHATAY_TOOLCHAIN_IMAGE:-chatay-resilientdb-toolchain:bazel6-20260528}"
BAZEL_CACHE_VOLUME="${CHATAY_BAZEL_CACHE_VOLUME:-chatay-bazel-cache-v215}"
BAZEL_JOBS="${BAZEL_JOBS:-4}"

ROOT="$(git rev-parse --show-toplevel)"
VERSION_DIR="${ROOT}/documents/chatay-porting/${VERSION}"
VERSION_REL="documents/chatay-porting/${VERSION}"
RUNTIME_CONTEXT="${VERSION_DIR}/runtime-context"
LOG_DIR="${VERSION_DIR}/logs"
SOURCE_COMMIT="$(git -C "${ROOT}" rev-parse HEAD)"
SOURCE_BRANCH="$(git -C "${ROOT}" rev-parse --abbrev-ref HEAD)"

mkdir -p "${RUNTIME_CONTEXT}/bin" "${LOG_DIR}"
cat > "${RUNTIME_CONTEXT}/.gitignore" <<'EOF'
*
!.gitignore
!chatay-runtime-manifest.json
!chatay-runtime-files.txt
EOF
cat > "${LOG_DIR}/.gitignore" <<'EOF'
*
!.gitignore
EOF

log() {
  printf '[%s] %s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$*"
}

log "building PBFT runtime binaries with ${TOOLCHAIN_IMAGE}"
docker run --rm \
  -v "${ROOT}:/workspace" \
  -w /workspace \
  -v "${BAZEL_CACHE_VOLUME}:/root/.cache/bazel" \
  "${TOOLCHAIN_IMAGE}" \
  bash -lc "set -euo pipefail; bazel build --jobs=${BAZEL_JOBS} //benchmark/protocols/pbft:kv_service //service/tools/kv/api_tools:kv_service_tools //tools:certificate_tools //tools:key_generator_tools //tools:generate_region_config; rm -rf ${VERSION_REL}/runtime-context/bin; mkdir -p ${VERSION_REL}/runtime-context/bin; cp bazel-bin/benchmark/protocols/pbft/kv_service ${VERSION_REL}/runtime-context/bin/kv_service; cp bazel-bin/service/tools/kv/api_tools/kv_service_tools ${VERSION_REL}/runtime-context/bin/kv_service_tools; cp bazel-bin/tools/certificate_tools ${VERSION_REL}/runtime-context/bin/certificate_tools; cp bazel-bin/tools/key_generator_tools ${VERSION_REL}/runtime-context/bin/key_generator_tools; cp bazel-bin/tools/generate_region_config ${VERSION_REL}/runtime-context/bin/generate_region_config; chmod +x ${VERSION_REL}/runtime-context/bin/*" \
  2>&1 | tee "${LOG_DIR}/bazel-build.log"

(
  cd "${RUNTIME_CONTEXT}"
  find bin -maxdepth 2 -type f -printf '%p %s bytes\n' | sort
) > "${RUNTIME_CONTEXT}/chatay-runtime-files.txt"

cat > "${RUNTIME_CONTEXT}/chatay-runtime-manifest.json" <<EOF
{
  "schemaVersion": "chatay-consensus-runtime-image-v2",
  "protocol": "pbft",
  "semver": "${VERSION}",
  "sourceCommit": "${SOURCE_COMMIT}",
  "sourceBranch": "${SOURCE_BRANCH}",
  "baseImage": "${BASE_IMAGE}",
  "runtimeWorkdir": "/opt/resilientdb-pbft",
  "binaries": [
    "kv_service",
    "kv_service_tools",
    "certificate_tools",
    "key_generator_tools",
    "generate_region_config"
  ]
}
EOF

log "building runtime image ${IMAGE} from ${BASE_IMAGE}"
docker build \
  -f "${ROOT}/tools/chatay/images/Dockerfile.pbft-runtime" \
  --build-arg BASE_IMAGE="${BASE_IMAGE}" \
  --build-arg CHATAY_SEMVER="${VERSION}" \
  --build-arg SOURCE_COMMIT="${SOURCE_COMMIT}" \
  --build-arg SOURCE_BRANCH="${SOURCE_BRANCH}" \
  -t "${IMAGE}" \
  "${VERSION_DIR}" \
  2>&1 | tee "${LOG_DIR}/docker-build.log"

image_id="$(docker image inspect "${IMAGE}" --format '{{.Id}}')"
image_size="$(docker image inspect "${IMAGE}" --format '{{.Size}}')"
image_labels="$(docker image inspect "${IMAGE}" --format '{{json .Config.Labels}}')"

cat > "${VERSION_DIR}/image-summary.json" <<EOF
{
  "schemaVersion": "chatay-consensus-runtime-image-summary-v1",
  "image": "${IMAGE}",
  "imageId": "${image_id}",
  "sizeBytes": ${image_size},
  "baseImage": "${BASE_IMAGE}",
  "sourceCommit": "${SOURCE_COMMIT}",
  "sourceBranch": "${SOURCE_BRANCH}",
  "labels": ${image_labels}
}
EOF

docker run --rm --entrypoint sh "${IMAGE}" -lc \
  'cat /chatay-runtime-manifest.json; printf "\n--- runtime files ---\n"; cat /chatay-runtime-files.txt; printf "\n--- bin listing ---\n"; ls -lh /opt/resilientdb-pbft/bin' \
  > "${VERSION_DIR}/runtime-content.txt"

log "runtime image built image=${IMAGE} id=${image_id} sizeBytes=${image_size}"
log "summary=${VERSION_DIR}/image-summary.json"
