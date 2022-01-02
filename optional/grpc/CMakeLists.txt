project(egel_rpc)
cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-g -Wall -Werror -fmodules-ts")

find_package(Protobuf REQUIRED)
find_package(ICU 60.0 REQUIRED COMPONENTS data i18n io tu uc)

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER src/egel_rpc.proto)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${PROTOBUF_INCLUDE_DIR})
include_directories(/usr/local/include)
include_directories(${ICU_INCLUDE_DIRS})
message( ${ICU_INCLUDE_DIRS} )

add_library(egel_rpc MODULE ${PROTO_HEADER} ${PROTO_SRC} src/egel_rpc.cpp)
target_link_libraries(egel_rpc proto ${PROTOBUF_LIBRARY})
target_link_libraries(egel_rpc ICU::uc ICU::i18n ICU::io)
set_target_properties(egel_rpc PROPERTIES PREFIX "" SUFFIX ".ego")

include(GNUInstallDirs)
install(TARGETS egel_rpc
    DESTINATION  "${CMAKE_INSTALL_LIBDIR}/egel"
)