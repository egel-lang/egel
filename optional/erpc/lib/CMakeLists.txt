#
# Dependencies
#
find_package(Threads)
find_package(ICU 60.0 REQUIRED COMPONENTS data i18n io tu uc)
include_directories(/usr/local/include) # compile against a local install

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#
# Sources
#
set(SOURCES
    egel_rpc.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

#
# Target
#
add_library(erpc SHARED ${SOURCES})
target_link_libraries(erpc
    PUBLIC
        ICU::uc ICU::i18n ICU::io
    PRIVATE
        egel-proto
)
set_target_properties(erpc PROPERTIES PREFIX "" SUFFIX ".ego")

# installation
include(GNUInstallDirs)

install(TARGETS erpc
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/egel"
)
