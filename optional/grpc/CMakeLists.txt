cmake_minimum_required(VERSION 3.5.1)

project(EgelRpc C CXX)

include(../cmake/common.cmake)

# Proto file
get_filename_component(egel_proto "../../protos/egel_rpc.proto" ABSOLUTE)
get_filename_component(egel_proto_path "${egel_proto}" PATH)

# Generated sources
set(egel_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/egel_rpc.pb.cc")
set(egel_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/egel_rpc.pb.h")
set(egel_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/egel_rpc.grpc.pb.cc")
set(egel_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/egel_rpc.grpc.pb.h")
add_custom_command(
      OUTPUT "${egel_proto_srcs}" "${egel_proto_hdrs}" "${egel_grpc_srcs}" "${egel_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${egel_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${egel_proto}"
      DEPENDS "${egel_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# egel_grpc_proto
add_library(egel_grpc_proto
  ${egel_grpc_srcs}
  ${egel_grpc_hdrs}
  ${egel_proto_srcs}
  ${egel_proto_hdrs})
target_link_libraries(egel_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# egel_rpc_helper
add_library(egel_rpc_helper
  "helper.h"
  "helper.cc")
target_link_libraries(egel_rpc_helper
  egel_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# Targets egel_rpc_(client|server)
foreach(_target
  egel_rpc_client egel_rpc_server)
  add_executable(${_target}
    "${_target}.cc")
  target_link_libraries(${_target}
    egel_grpc_proto
    egel_rpc_helper
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()
