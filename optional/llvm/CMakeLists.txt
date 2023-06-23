cmake_minimum_required(VERSION 3.13)
project(llvm LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# Set LLVM_DIR to the directory containing LLVMConfig.cmake
#set(LLVM_DIR /path/to/llvm/cmake/modules/CMakeFiles)

find_package(LLVM REQUIRED CONFIG)
find_package(ICU 60.0 REQUIRED COMPONENTS data i18n io tu uc)

include_directories(/usr/local/include) # compile against a local install
# Add LLVM include directories
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Set additional flags for LLVM IR compilation
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_COMPILE_FLAGS}")

#
# Sources
#
set(SOURCES
    src/llvm.cpp
)

#
# Target
#
add_library(llvm SHARED ${SOURCES})
set_target_properties(llvm PROPERTIES PREFIX "" SUFFIX ".ego")

# Link against LLVM libraries
llvm_map_components_to_libnames(LLVM_LIBS support core irreader)

target_link_libraries(llvm PUBLIC ICU::uc ICU::i18n ICU::io)
target_link_libraries(llvm ${LLVM_LIBS})

#
# installation
#
include(GNUInstallDirs)

install(TARGETS llvm
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/egel"
)
