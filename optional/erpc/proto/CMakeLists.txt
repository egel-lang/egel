find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads)
find_package(ICU 60.0 REQUIRED COMPONENTS data i18n io tu uc)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#
# Protobuf/Grpc source files
#
set(PROTO_FILES
    egel.proto
)

#
# Add Library target with protobuf sources
#
add_library(egel-proto ${PROTO_FILES})
target_link_libraries(egel-proto
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(egel-proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

#
# Compile protobuf and grpc files in egel-proto target to cpp
#
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET egel-proto LANGUAGE cpp)
protobuf_generate(TARGET egel-proto LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")
