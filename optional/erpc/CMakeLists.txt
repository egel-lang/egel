cmake_minimum_required(VERSION 3.13)
project(egel-rpc LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib)
add_subdirectory(proto)
