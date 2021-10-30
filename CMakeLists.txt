# minimal cmake version
cmake_minimum_required(VERSION 3.13)

# fallback for other versions 
if(${CMAKE_VERSION} VERSION_LESS 3.18)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.18)
endif()

# set the cxx compiler standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# steal the version of the interpreter
set(VERSION_MAJOR_REGEX "#define EXECUTABLE_VERSION_MAJOR[ \t]+\"(.+)\"")
set(VERSION_MINOR_REGEX "#define EXECUTABLE_VERSION_MINOR[ \t]+\"(.+)\"")
set(VERSION_PATCH_REGEX "#define EXECUTABLE_VERSION_PATCH[ \t]+\"(.+)\"")

file(STRINGS "${CMAKE_SOURCE_DIR}/src/egel.cpp"
    VERSION_MAJOR REGEX ${VERSION_MAJOR_REGEX})
file(STRINGS "${CMAKE_SOURCE_DIR}/src/egel.cpp"
    VERSION_MINOR REGEX ${VERSION_MINOR_REGEX})
file(STRINGS "${CMAKE_SOURCE_DIR}/src/egel.cpp"
    VERSION_PATCH REGEX ${VERSION_PATCH_REGEX})

string(REGEX REPLACE ${VERSION_MAJOR_REGEX} "\\1" VERSION_MAJOR "${VERSION_MAJOR}")
string(REGEX REPLACE ${VERSION_MINOR_REGEX} "\\1" VERSION_MINOR "${VERSION_MINOR}")
string(REGEX REPLACE ${VERSION_PATCH_REGEX} "\\1" VERSION_PATCH "${VERSION_PATCH}")

set(PVERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

# steal the search path of the interpreter
set(INCLUDE_PATH_REGEX "#define INCLUDE_PATH[ \t]+\"(.+)\"")

file(STRINGS "${CMAKE_SOURCE_DIR}/src/egel.cpp"
    INCLUDE_PATH REGEX ${INCLUDE_PATH_REGEX})

string(REGEX REPLACE ${INCLUDE_PATH_REGEX} "\\1" INCLUDE_PATH "${INCLUDE_PATH}")

# add 'fallback' modules
set(CMAKE_MODULE_PATH "contrib/cmake/modules;${CMAKE_MODULE_PATH}")

# the Egel project
project(Egel DESCRIPTION "The Egel interpreter"
        VERSION ${PVERSION} LANGUAGES CXX)

# set the cxx compiler flags
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# compilers
message("compiler: ${CMAKE_CXX_COMPILER_ID}")
if(("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"))
  set(CMAKE_CXX_FLAGS                     "-Wall -Wextra -Werror -pedantic -Wno-unused-parameter -Wno-return-type-c-linkage")
  set(CMAKE_CXX_FLAGS_DEBUG               "-g")
  set(CMAKE_CXX_FLAGS_RELEASE             "-Ofast")
elseif(("${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang"))
  set(CMAKE_CXX_FLAGS                     "-Wall -Wextra -Werror -pedantic -Wno-unused-parameter -Wno-return-type-c-linkage")
  set(CMAKE_CXX_FLAGS_DEBUG               "-g")
  set(CMAKE_CXX_FLAGS_RELEASE             "-Ofast")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS                     "-Wall -Wextra -Werror -pedantic -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS_DEBUG               "-g")
  set(CMAKE_CXX_FLAGS_RELEASE             "-O3")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  set(CMAKE_CXX_FLAGS                     "-Wall -Wextra -Werror -pedantic -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS_DEBUG               "-g")
  set(CMAKE_CXX_FLAGS_RELEASE             "-O3")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS                     "-Wall -Wextra -Werror -pedantic -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS_DEBUG               "-g")
  set(CMAKE_CXX_FLAGS_RELEASE             "-O3")
else()
  set(CMAKE_CXX_FLAGS                     "-Wall -Wextra -Werror -pedantic -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS_DEBUG               "-g")
  set(CMAKE_CXX_FLAGS_RELEASE             "-O3")
endif()

SET(CMAKE_CXX_FLAGS_PROFILE             "-O3 -pg")
SET(CMAKE_EXE_LINKER_FLAGS_PROFILE      "-O3 -pg")
SET(CMAKE_SHARED_LINKER_FLAGS_PROFILE   "-O3 -pg")


# for the moment, glob sources and headers
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.hpp")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

# check for ICU, Threads, fmt
# ICU components: data, i18n, io, le, lx, test, tu and uc.
find_package(ICU 60.0 REQUIRED COMPONENTS data i18n io tu uc)
find_package(Threads REQUIRED)
find_package(fmt 7.1 REQUIRED)

include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/src/builtin")

# objects
add_library(objlib OBJECT ${SOURCE_LIST})
set_property(TARGET objlib PROPERTY POSITION_INDEPENDENT_CODE 1)
target_link_libraries(objlib fmt::fmt Threads::Threads ICU::uc ICU::i18n ICU::io)

# the Egel interpreter
add_executable(egel $<TARGET_OBJECTS:objlib>)
target_link_libraries(egel ${CMAKE_DL_LIBS} fmt::fmt Threads::Threads ICU::uc ICU::i18n ICU::io)


# shared Egel library
add_library(egellib SHARED $<TARGET_OBJECTS:objlib>)
set_target_properties(egellib PROPERTIES OUTPUT_NAME egel)
target_link_libraries(egellib fmt::fmt Threads::Threads ICU::uc ICU::i18n ICU::io)

# static Egel library
add_library(egellib_static STATIC $<TARGET_OBJECTS:objlib>)
set_target_properties(egellib_static PROPERTIES OUTPUT_NAME egel)
target_link_libraries(egellib_static fmt::fmt Threads::Threads ICU::uc ICU::i18n ICU::io)

#
# the dynamic .ego Egel libraries

# os.ego
file(GLOB OS_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/lib/os/*.cpp")
add_library(os MODULE ${OS_LIST})
target_link_libraries(os ICU::uc ICU::i18n ICU::io)
set_target_properties(os PROPERTIES PREFIX "" SUFFIX ".ego")

# fs.ego
file(GLOB FS_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/lib/fs/*.cpp")
add_library(fs MODULE ${FS_LIST})
target_link_libraries(fs ICU::uc ICU::i18n ICU::io)
set_target_properties(fs PROPERTIES PREFIX "" SUFFIX ".ego")

# regex.ego
file(GLOB REGEX_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/lib/regex/*.cpp")
add_library(regex MODULE ${REGEX_LIST})
target_link_libraries(regex ICU::uc ICU::i18n ICU::io)
set_target_properties(regex PROPERTIES PREFIX "" SUFFIX ".ego")

# random.ego
file(GLOB RANDOM_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/lib/random/*.cpp")
add_library(random MODULE ${RANDOM_LIST})
target_link_libraries(random ICU::uc ICU::i18n ICU::io)
set_target_properties(random PROPERTIES PREFIX "" SUFFIX ".ego")

# installation
include(GNUInstallDirs)

install(TARGETS egel
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS egellib egellib_static
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${HEADER_LIST}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/egel
)

file(GLOB INCLUDE_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/*.eg")
install(FILES ${INCLUDE_LIST}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS os fs regex random
	LIBRARY DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES "${CMAKE_SOURCE_DIR}/man/egel.1"
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
)
