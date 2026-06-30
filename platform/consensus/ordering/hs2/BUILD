package(default_visibility = ["//visibility:public"])

cc_library(
    name = "hs2_core",
    srcs = ["hs2_consensus.cpp"],
    hdrs = [
        "hs2_consensus.h",
        "hs2_types.h",
    ],
)

cc_library(
    name = "consensus_manager_hs2",
    srcs = ["consensus_manager_hs2.cpp"],
    hdrs = ["consensus_manager_hs2.h"],
    deps = [
        ":hs2_core",
        ":hs2_new_txn_pipeline",
        "//executor/common:custom_query",
        "//executor/common:transaction_manager",
        "//platform/consensus/execution:system_info",
        "//platform/consensus/ordering/common:commitment_basic",
        ":hs2_pacemaker",
        "//platform/consensus/ordering/common:message_manager_basic",
        "//platform/consensus/ordering/common:response_manager",
        "//platform/config:resdb_config",
        "//platform/networkstrate:consensus_manager",
        "//platform/proto:resdb_cc_proto",
    ],
    visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "hs2_new_txn_pipeline",
    srcs = ["hs2_new_txn_pipeline.cpp"],
    hdrs = ["hs2_new_txn_pipeline.h"],
    deps = [
        ":hs2_core",
        ":hs2_payload_binding",
        ":hs2_qc_boundary",
        "//common/crypto:signature_verifier",
        "//platform/config:resdb_config",
        "//platform/proto:resdb_cc_proto",
    ],
    visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "hs2_payload_binding",
    srcs = ["hs2_payload_binding.cpp"],
    hdrs = ["hs2_payload_binding.h"],
    deps = [
        ":hs2_core",
        "//common/crypto:signature_verifier",
        "//platform/proto:resdb_cc_proto",
    ],
    visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "hs2_qc_boundary",
    srcs = ["hs2_qc_boundary.cpp"],
    hdrs = ["hs2_qc_boundary.h"],
    deps = [
        ":hs2_core",
        "//common/crypto:signature_verifier",
    ],
    visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "hs2_pacemaker",
    srcs = ["hs2_pacemaker.cpp"],
    hdrs = ["hs2_pacemaker.h"],
    deps = [
        "//platform/config:resdb_config",
        "//platform/consensus/execution:system_info",
        "//platform/proto:resdb_cc_proto",
    ],
    visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "hs2_timeout_certificate",
    srcs = ["hs2_timeout_certificate.cpp"],
    hdrs = ["hs2_timeout_certificate.h"],
    deps = [
        ":hs2_core",
    ],
    visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "hs2_new_view",
    srcs = ["hs2_new_view.cpp"],
    hdrs = ["hs2_new_view.h"],
    deps = [
        ":hs2_core",
        ":hs2_timeout_certificate",
    ],
    visibility = [
        "//visibility:public",
    ],
)
