cmake_minimum_required(VERSION 3.20)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Do not build in-source. Please remove CMakeCache.txt and the CMakeFiles/ directory. Then build out-of-source.")
endif()

# WORKAROUND
# This is added so that CMake can recognize the .ixx extension as a module interface.
set(CMAKE_CXX_SYSROOT_FLAG_CODE "list(APPEND CMAKE_CXX_SOURCE_FILE_EXTENSIONS ixx)")

project(
    egel
    VERSION 0.1.18
    LANGUAGES CXX
)

# It is always easier to navigate in an IDE when projects are organized in folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

message("CMAKE_CXX_SOURCE_FILE_EXTENSIONS ${CMAKE_CXX_SOURCE_FILE_EXTENSIONS}")

find_package(ICU 60.0 REQUIRED COMPONENTS data i18n io tu uc)

include_directories(
    src
)



add_executable(egel
    src/constants.ixx
    src/utils.ixx
    src/position.ixx
)
target_link_libraries(egel ICU::uc ICU::i18n )

# Only support statically linking for now.
target_compile_definitions(egel PRIVATE
                            SEQUENTIAL_PROCESSING=0)


# Require c++20, this is better than setting CMAKE_CXX_STANDARD since it won't pollute other targets
# note : cxx_std_* features were added in CMake 3.8.2
target_compile_features(egel PRIVATE cxx_std_20)

target_compile_options(egel PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4 /WX /permissive- /Z7 /MT /MP /Bt+>)

set_target_properties(egel PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)