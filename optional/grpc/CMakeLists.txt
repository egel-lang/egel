project(egel_rpc)
cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-g -Wall -Werror -fmodules-ts")

find_package(Protobuf REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")
find_package(GRPC REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
find_package(ICU 60.0 REQUIRED COMPONENTS data i18n io tu uc)
message(STATUS "Using ICU ${ICU_VERSION}")

get_target_property(gRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin
    IMPORTED_LOCATION_RELEASE)

get_filename_component(EGEL_PROTO "src/egel_rpc.proto" ABSOLUTE)
get_filename_component(EGEL_PROTO_PATH "${EGEL_PROTO}" PATH)

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER ${EGEL_PROTO})
set(EGEL_GRPC_SRCS "${CMAKE_CURRENT_BINARY_DIR}/egel_rpc.grpc.pb.cc")
set(EGEL_GRPC_HDRS "${CMAKE_CURRENT_BINARY_DIR}/egel_rpc.grpc.pb.h")
add_custom_command(
      OUTPUT "${EGEL_GRPC_SRCS}" "${EGEL_GRPC_HDRS}"
      COMMAND protobuf::protoc
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}" -I "${EGEL_PROTO_PATH}"
        --plugin=protoc-gen-grpc="${gRPC_CPP_PLUGIN_EXECUTABLE}"
        "${EGEL_PROTO}"
      DEPENDS "${EGEL_PROTO}")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${PROTOBUF_INCLUDE_DIR})
include_directories(/usr/local/include)
include_directories(${ICU_INCLUDE_DIRS})
message("including ICU: ${ICU_INCLUDE_DIRS}")

add_library(egel_rpc MODULE ${PROTO_HEADER} ${PROTO_SRC} ${EGEL_GRPC_HDRS} ${EGEL_GRPC_SRCS} src/egel_rpc.cpp)
target_link_libraries(egel_rpc proto ${PROTOBUF_LIBRARY})
target_link_libraries(egel_rpc ICU::uc ICU::i18n ICU::io)
set_target_properties(egel_rpc PROPERTIES PREFIX "" SUFFIX ".ego")

include(GNUInstallDirs)
install(TARGETS egel_rpc
    DESTINATION  "${CMAKE_INSTALL_LIBDIR}/egel"
)
