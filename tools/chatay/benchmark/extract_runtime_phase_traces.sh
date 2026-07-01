#!/usr/bin/env bash

set -Eeuo pipefail

VERSION="${CHATAY_SEMVER:-v2.18.5-rc.1}"
ROOT="$(git rev-parse --show-toplevel)"
SOURCE_LOG_ROOT="${CHATAY_SOURCE_LOG_ROOT:-${ROOT}/documents/chatay-porting/v2.18.4-beta.1/logs/20260701T021524Z-runtime-coldstart}"
SOURCE_LOG_ROOT_LABEL="${CHATAY_SOURCE_LOG_ROOT_LABEL:-documents/chatay-porting/v2.18.4-beta.1/logs/20260701T021524Z-runtime-coldstart}"
VERSION_DIR="${ROOT}/documents/chatay-porting/${VERSION}"
EVENTS_CSV="${VERSION_DIR}/phase-trace-events.csv"
COVERAGE_CSV="${VERSION_DIR}/phase-trace-coverage.csv"
COVERAGE_MD="${VERSION_DIR}/PHASE_TRACE_COVERAGE.md"

mkdir -p "${VERSION_DIR}"

python3 - "${SOURCE_LOG_ROOT}" "${EVENTS_CSV}" "${COVERAGE_CSV}" "${COVERAGE_MD}" "${SOURCE_LOG_ROOT_LABEL}" <<'PY'
import csv
import os
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

source_root = Path(sys.argv[1])
events_csv = Path(sys.argv[2])
coverage_csv = Path(sys.argv[3])
coverage_md = Path(sys.argv[4])
source_label = sys.argv[5]

protocols = ("pbft", "hs1-pr100", "hs2")

patterns = [
    ("config_load", re.compile(r"CHATAY_HS1_COLD_START (generate_resdb_config_start|read_config_start|read_config_finish|generate_resdb_config_finish)")),
    ("key_cert_load", re.compile(r"CHATAY_HS1_COLD_START (read_private_key_|read_cert_|key_cert_loaded)")),
    ("network_start", re.compile(r"CHATAY_HS1_COLD_START (service_network_ctor_start|service_network_service_started|service_network_acceptor_run_enter)")),
    ("readiness", re.compile(r"(CHATAY_HS1_COLD_START readiness_reached|Server [0-9]+ is ready)")),
    ("bootstrap_heartbeat_send", re.compile(r"CHATAY_HS1_COLD_START heartbeat_send_start")),
    ("bootstrap_heartbeat_receive", re.compile(r"CHATAY_HS1_COLD_START heartbeat_receive_enter")),
    ("pbft_commit", re.compile(r"message_manager\.cpp:188].*has been committed")),
    ("hs1_consensus_commit", re.compile(r"CHATAY_HS1_TRACE consensus_commit")),
    ("hs1_prepare_vote", re.compile(r"CHATAY_HS1_TRACE replica_send_vote .*vote_type=TYPE_PREPARE_VOTE")),
    ("hs1_precommit_vote", re.compile(r"CHATAY_HS1_TRACE replica_send_vote .*vote_type=TYPE_PRECOMMIT_VOTE")),
    ("hs1_commit_vote", re.compile(r"CHATAY_HS1_TRACE replica_send_vote .*vote_type=TYPE_COMMIT_VOTE")),
    ("hs2_phase2_certified", re.compile(r"CHATAY_HS2_PIPELINE certified")),
    ("client_set_ok", re.compile(r"client set ret = 0")),
    ("client_get_ok", re.compile(r"client get value = ")),
]

expected = {
    "pbft": [
        "config_load",
        "key_cert_load",
        "network_start",
        "readiness",
        "bootstrap_heartbeat_send",
        "bootstrap_heartbeat_receive",
        "pbft_commit",
        "client_set_ok",
        "client_get_ok",
    ],
    "hs1-pr100": [
        "config_load",
        "key_cert_load",
        "network_start",
        "readiness",
        "bootstrap_heartbeat_send",
        "bootstrap_heartbeat_receive",
        "hs1_consensus_commit",
        "hs1_prepare_vote",
        "hs1_precommit_vote",
        "hs1_commit_vote",
        "client_set_ok",
        "client_get_ok",
    ],
    "hs2": [
        "config_load",
        "key_cert_load",
        "network_start",
        "readiness",
        "bootstrap_heartbeat_send",
        "bootstrap_heartbeat_receive",
        "hs2_phase2_certified",
        "client_set_ok",
        "client_get_ok",
    ],
}

def detect_protocol(path: Path) -> str:
    parts = set(path.parts)
    for protocol in protocols:
        if protocol in parts:
            return protocol
    return "unknown"

def detect_repeat(path: Path) -> str:
    for part in path.parts:
        if part.startswith("repeat-"):
            return part
    return ""

def detect_node(path: Path) -> str:
    match = re.search(r"node-(\d+)\.log$", path.name)
    if match:
        return match.group(1)
    if path.name.startswith("client-"):
        return "client"
    return ""

def extract_ts_ms(line: str) -> str:
    match = re.search(r"ts_ms=(\d+)", line)
    if match:
        return match.group(1)
    match = re.search(r"^[EIWF](\d{8})\s+(\d{2}:\d{2}:\d{2}\.\d+)", line)
    if match:
        return f"{match.group(1)} {match.group(2)}"
    return ""

events = []
counts = Counter()

for path in sorted(source_root.rglob("*")):
    if path.suffix not in (".log", ".trace"):
        continue
    protocol = detect_protocol(path)
    if protocol == "unknown":
        continue
    repeat = detect_repeat(path)
    node = detect_node(path)
    rel = path.relative_to(source_root).as_posix()
    try:
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError:
        continue
    for line_no, line in enumerate(lines, start=1):
        for category, pattern in patterns:
            if pattern.search(line):
                counts[(protocol, category)] += 1
                events.append({
                    "protocol": protocol,
                    "repeat": repeat,
                    "node": node,
                    "category": category,
                    "ts": extract_ts_ms(line),
                    "source": rel,
                    "line": line_no,
                    "message": line[:500],
                })
                break

events_csv.parent.mkdir(parents=True, exist_ok=True)
with events_csv.open("w", newline="", encoding="utf-8") as fh:
    writer = csv.DictWriter(fh, fieldnames=["protocol", "repeat", "node", "category", "ts", "source", "line", "message"])
    writer.writeheader()
    writer.writerows(events)

coverage_rows = []
for protocol in protocols:
    for category in expected[protocol]:
        count = counts[(protocol, category)]
        coverage_rows.append({
            "protocol": protocol,
            "category": category,
            "status": "OBSERVED" if count else "NOT_OBSERVED",
            "count": count,
        })

for protocol in protocols:
    coverage_rows.append({
        "protocol": protocol,
        "category": "view_change_or_leader_failure",
        "status": "NOT_EXECUTED",
        "count": 0,
    })

with coverage_csv.open("w", newline="", encoding="utf-8") as fh:
    writer = csv.DictWriter(fh, fieldnames=["protocol", "category", "status", "count"])
    writer.writeheader()
    writer.writerows(coverage_rows)

by_protocol = defaultdict(int)
for event in events:
    by_protocol[event["protocol"]] += 1

with coverage_md.open("w", encoding="utf-8") as fh:
    fh.write("# Runtime Phase Trace Coverage - v2.18.5-rc.1\n\n")
    fh.write(f"Source log root: `{source_label}`\n\n")
    fh.write("## Event Counts\n\n")
    fh.write("| Protocol | Events extracted |\n")
    fh.write("| --- | ---: |\n")
    for protocol in protocols:
        fh.write(f"| `{protocol}` | {by_protocol[protocol]} |\n")
    fh.write("\n## Coverage Matrix\n\n")
    fh.write("| Protocol | Category | Status | Count |\n")
    fh.write("| --- | --- | --- | ---: |\n")
    for row in coverage_rows:
        fh.write(f"| `{row['protocol']}` | `{row['category']}` | `{row['status']}` | {row['count']} |\n")
    fh.write("\n## Claim Boundary\n\n")
    fh.write("The extracted events are runtime log observations from a local no-fault run. ")
    fh.write("They confirm observable startup, readiness, selected commit/certification markers and client-visible SET/GET success. ")
    fh.write("They do not prove complete protocol conformance and do not include a real view-change or leader-failure scenario in this gate.\n")

print(f"events={events_csv}")
print(f"coverage={coverage_csv}")
print(f"coverage_md={coverage_md}")
PY
