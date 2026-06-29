# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#

package(default_visibility = ["//platform:__subpackages__"])

cc_library(
    name = "transaction_utils",
    srcs = ["transaction_utils.cpp"],
    hdrs = ["transaction_utils.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//platform/proto:resdb_cc_proto",
    ],
)

cc_library(
    name = "atomic_unique_ptr",
    hdrs = ["atomic_unique_ptr.h"],
)

cc_library(
    name = "message_manager_basic",
    srcs = ["message_manager_basic.cpp"],
    hdrs = ["message_manager_basic.h"],
    deps = [
        "//executor/common:transaction_manager",
        "//platform/common/queue:lock_free_queue",
        "//platform/config:resdb_config",
        "//platform/consensus/execution:system_info",
        "//platform/consensus/execution:transaction_executor",
        "//platform/proto:resdb_cc_proto",
    ],
)

cc_library(
    name = "commitment_basic",
    srcs = ["commitment_basic.cpp"],
    hdrs = ["commitment_basic.h"],
    deps = [
        ":message_manager_basic",
        "//common/crypto:signature_verifier",
        "//common/utils",
        "//platform/config:resdb_config",
        "//platform/networkstrate:replica_communicator",
        "//platform/networkstrate:server_comm",
        "//platform/statistic:stats",
    ],
)

cc_library(
    name = "response_manager",
    srcs = ["response_manager.cpp"],
    hdrs = ["response_manager.h"],
    deps = [
        ":transaction_utils",
        "//common/crypto:signature_verifier",
        "//common/utils",
        "//platform/common/queue:lock_free_queue",
        "//platform/config:resdb_config",
        "//platform/consensus/execution:system_info",
        "//platform/networkstrate:replica_communicator",
        "//platform/networkstrate:server_comm",
        "//platform/proto:resdb_cc_proto",
        "//platform/statistic:stats",
    ],
)

cc_library(
    name = "performance_manager",
    srcs = ["performance_manager.cpp"],
    hdrs = ["performance_manager.h"],
    deps = [
        ":transaction_utils",
        "//common/crypto:signature_verifier",
        "//common/utils",
        "//platform/common/queue:lock_free_queue",
        "//platform/config:resdb_config",
        "//platform/consensus/execution:system_info",
        "//platform/networkstrate:replica_communicator",
        "//platform/networkstrate:server_comm",
        "//platform/proto:resdb_cc_proto",
        "//platform/statistic:stats",
    ],
)
