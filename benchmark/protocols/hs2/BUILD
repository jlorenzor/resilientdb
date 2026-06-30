cc_binary(
    name = "hs2_skeleton_probe",
    srcs = ["hs2_skeleton_probe.cpp"],
    deps = [
        "//platform/consensus/ordering/hs2:hs2_core",
    ],
)

cc_binary(
    name = "hs2_safety_probe",
    srcs = ["hs2_safety_probe.cpp"],
    deps = [
        "//platform/consensus/ordering/hs2:hs2_core",
    ],
)

cc_binary(
    name = "hs2_wiring_probe",
    srcs = ["hs2_wiring_probe.cpp"],
    deps = [
        "//platform/config:resdb_config",
        "//platform/consensus/ordering/hs2:consensus_manager_hs2",
    ],
)

cc_binary(
    name = "kv_service",
    srcs = ["kv_service.cpp"],
    deps = [
        "//chain/storage:memory_db",
        "//executor/kv:kv_executor",
        "//platform/config:resdb_config_utils",
        "//platform/consensus/ordering/hs2:consensus_manager_hs2",
        "//platform/networkstrate:service_network",
    ],
    visibility = [
        "//visibility:public",
    ],
)

cc_binary(
    name = "hs2_pacemaker_probe",
    srcs = ["hs2_pacemaker_probe.cpp"],
    deps = [
        "//platform/config:resdb_config",
        "//platform/consensus/ordering/hs2:hs2_pacemaker",
    ],
)

cc_binary(
    name = "hs2_timeout_certificate_probe",
    srcs = ["hs2_timeout_certificate_probe.cpp"],
    deps = [
        "//platform/config:resdb_config",
        "//platform/consensus/ordering/hs2:hs2_pacemaker",
        "//platform/consensus/ordering/hs2:hs2_timeout_certificate",
    ],
)

cc_binary(
    name = "hs2_new_view_highqc_probe",
    srcs = ["hs2_new_view_highqc_probe.cpp"],
    deps = [
        "//platform/config:resdb_config",
        "//platform/consensus/ordering/hs2:hs2_core",
        "//platform/consensus/ordering/hs2:hs2_new_view",
        "//platform/consensus/ordering/hs2:hs2_pacemaker",
        "//platform/consensus/ordering/hs2:hs2_timeout_certificate",
    ],
)

cc_binary(
    name = "hs2_negative_safety_probe",
    srcs = ["hs2_negative_safety_probe.cpp"],
    deps = [
        "//platform/config:resdb_config",
        "//platform/consensus/ordering/hs2:hs2_core",
        "//platform/consensus/ordering/hs2:hs2_new_view",
        "//platform/consensus/ordering/hs2:hs2_pacemaker",
        "//platform/consensus/ordering/hs2:hs2_timeout_certificate",
    ],
)
