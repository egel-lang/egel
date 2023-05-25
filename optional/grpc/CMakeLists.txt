project(egel_rpc)
cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-g -Wall -Werror -fmodules-ts")

find_package(ICU 60.0 REQUIRED COMPONENTS data i18n io tu uc)
message(STATUS "Using ICU ${ICU_VERSION}")
find_package(Protobuf REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

get_filename_component(EGEL_PROTO "src/egel_rpc.proto" ABSOLUTE)
get_filename_component(EGEL_PROTO_PATH "${EGEL_PROTO}" PATH)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${EGEL_PROTO})
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS EXPORT_MACRO DLL_EXPORT ${EGEL_PROTO})
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS DESCRIPTORS PROTO_DESCS ${EGEL_PROTO})

include_directories(${PROTOBUF_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${ICU_INCLUDE_DIRS})
include_directories(../../..)
message("including ICU: ${ICU_INCLUDE_DIRS}")
message("including PROTOBUF: ${PROTOBUF_INCLUDE_DIRS}")

add_library(egel_rpc MODULE ${PROTO_SRCS} ${PROTO_HDRS} src/egel_rpc.cpp)
target_link_libraries(egel_rpc
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
        ICU::uc ICU::i18n ICU::io
)
set_target_properties(egel_rpc PROPERTIES PREFIX "" SUFFIX ".ego")

include(GNUInstallDirs)
install(TARGETS egel_rpc
    DESTINATION  "${CMAKE_INSTALL_LIBDIR}/egel"
)
