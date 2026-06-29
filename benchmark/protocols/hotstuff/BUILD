package(default_visibility = ["//visibility:private"])

cc_binary(
    name = "kv_server_performance",
    srcs = ["kv_server_performance.cpp"],
    deps = [
        "//executor/kv:kv_executor",
        "//platform/config:resdb_config_utils",
        "//platform/consensus/ordering/hotstuff:consensus",
        "//service/utils:server_factory",
    ],
)

cc_binary(
    name = "kv_service",
    srcs = ["kv_service.cpp"],
    deps = [
        "//chain/state:chain_state",
        "//executor/kv:kv_executor",
        "//platform/config:resdb_config_utils",
        "//platform/consensus/ordering/hotstuff:consensus",
        "//platform/networkstrate:service_network",
    ],
)
