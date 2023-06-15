#
# Dependencies
#
find_package(Threads)
find_package(ICU 60.0 REQUIRED COMPONENTS data i18n io tu uc)
include_directories(/usr/local/include) # compile against a local install

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
add_library(egel_rpc SHARED ${SOURCES})
target_link_libraries(egel_rpc
    PUBLIC
        ICU::uc ICU::i18n ICU::io
    PRIVATE
        egel-proto
)
set_target_properties(egel_rpc PROPERTIES PREFIX "" SUFFIX ".ego")
