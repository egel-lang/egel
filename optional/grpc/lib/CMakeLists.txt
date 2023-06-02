#
# Dependencies
#
find_package(Threads)

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
add_library(egel_rpc ${SOURCES})
target_link_libraries(egel_rpc
    PRIVATE
        egel-proto
)
