cmake_minimum_required(VERSION 3.13)
project(light LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(ICU 60.0 REQUIRED COMPONENTS data i18n io tu uc)

include_directories(/usr/local/include) # compile against a local install

#
# Sources
#
set(SOURCES
    src/light.cpp
)

#
# Target
#
add_library(light SHARED ${SOURCES})
set_target_properties(light PROPERTIES PREFIX "" SUFFIX ".ego")

target_link_libraries(light PUBLIC ICU::uc ICU::i18n ICU::io lightning)

#
# installation
#
include(GNUInstallDirs)

install(TARGETS light
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/egel"
)
