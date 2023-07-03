# minimal cmake version
cmake_minimum_required(VERSION 3.20)

# fallback for other versions 
if(${CMAKE_VERSION} VERSION_LESS 3.20)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.20) 
endif()

message("generator: ${CMAKE_GENERATOR}")

# set the cxx compiler standard
set(CMAKE_CXX_STANDARD 20)
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
set(EGEL_PATH_REGEX "#define EGEL_PATH[ \t]+\"(.+)\"")

file(STRINGS "${CMAKE_SOURCE_DIR}/src/egel.cpp"
    EGEL_PATH REGEX ${EGEL_PATH_REGEX})

string(REGEX REPLACE ${EGEL_PATH_REGEX} "\\1" EGEL_PATH "${EGEL_PATH}")

# report findings
message("egel version: ${PVERSION}")
message("egel path: ${EGEL_PATH}")

# add 'fallback' modules
# set(CMAKE_MODULE_PATH "contrib/cmake/modules;${CMAKE_MODULE_PATH}")

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

  set(CMAKE_CXX_FLAGS_PROFILE             "-Ofast -fprofile-instr-generate -fcoverage-mapping")
  #set(CMAKE_EXE_LINKER_FLAGS_PROFILE      "-O3 -pg")
  #set(CMAKE_SHARED_LINKER_FLAGS_PROFILE   "-O3 -pg")
elseif(("${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang"))
  set(CMAKE_CXX_FLAGS                     "-Wall -Wextra -Werror -pedantic -Wno-unused-parameter -Wno-return-type-c-linkage")
  set(CMAKE_CXX_FLAGS_DEBUG               "-g")
  set(CMAKE_CXX_FLAGS_RELEASE             "-Ofast")

  #set(CMAKE_CXX_FLAGS_PROFILE             "-O3 -pg")
  #set(CMAKE_EXE_LINKER_FLAGS_PROFILE      "-O3 -pg")
  #set(CMAKE_SHARED_LINKER_FLAGS_PROFILE   "-O3 -pg")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS                     "-Wall -Wextra -Werror -pedantic -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS_DEBUG               "-g")
  set(CMAKE_CXX_FLAGS_RELEASE             "-O3")

  #set(CMAKE_CXX_FLAGS_PROFILE             "-O3 -pg")
  #set(CMAKE_EXE_LINKER_FLAGS_PROFILE      "-O3 -pg")
  #set(CMAKE_SHARED_LINKER_FLAGS_PROFILE   "-O3 -pg")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  set(CMAKE_CXX_FLAGS                     "-Wall -Wextra -Werror -pedantic -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS_DEBUG               "-g")
  set(CMAKE_CXX_FLAGS_RELEASE             "-O3")

  set(CMAKE_CXX_FLAGS_PROFILE             "-O3 -pg")
  set(CMAKE_EXE_LINKER_FLAGS_PROFILE      "-O3 -pg")
  set(CMAKE_SHARED_LINKER_FLAGS_PROFILE   "-O3 -pg")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS                     "-Wall -Wextra -Werror -pedantic -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS_DEBUG               "-g")
  set(CMAKE_CXX_FLAGS_RELEASE             "-O3")

  set(CMAKE_CXX_FLAGS_PROFILE             "-O3 -pg")
  set(CMAKE_EXE_LINKER_FLAGS_PROFILE      "-O3 -pg")
  set(CMAKE_SHARED_LINKER_FLAGS_PROFILE   "-O3 -pg")
else()
  set(CMAKE_CXX_FLAGS                     "-Wall -Wextra -Werror -pedantic -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS_DEBUG               "-g")
  set(CMAKE_CXX_FLAGS_RELEASE             "-O3")

  set(CMAKE_CXX_FLAGS_PROFILE             "-O3 -pg")
  set(CMAKE_EXE_LINKER_FLAGS_PROFILE      "-O3 -pg")
  set(CMAKE_SHARED_LINKER_FLAGS_PROFILE   "-O3 -pg")
endif()

# for the moment, glob sources and headers
file(GLOB EGEL_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB EGEL_HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.hpp")

# check for ICU, Threads, fmt
# ICU components: data, i18n, io, le, lx, test, tu and uc.
find_package(ICU 60.0 REQUIRED COMPONENTS data i18n io tu uc)
find_package(Threads REQUIRED)
find_package(fmt 7.1 REQUIRED)
find_package(mimalloc 2.1 REQUIRED NO_SYSTEM_ENVIRONMENT_PATH)

include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${ICU_INCLUDE_DIRS}")
include_directories(/usr/local/include) ## for GNU lightning

# objects
add_library(objlib OBJECT ${EGEL_SOURCES})
set_property(TARGET objlib PROPERTY POSITION_INDEPENDENT_CODE 1)
target_link_libraries(objlib fmt::fmt Threads::Threads ICU::uc ICU::i18n ICU::io mimalloc-static)
target_link_directories(objlib PRIVATE /usr/local/lib) # for GNU lightning
target_link_options(objlib PRIVATE "-Wl,--allow-multiple-definition")

# the Egel interpreter
add_executable(egel $<TARGET_OBJECTS:objlib>)
target_link_libraries(egel ${CMAKE_DL_LIBS} fmt::fmt Threads::Threads ICU::uc ICU::i18n ICU::io mimalloc-static lightning)
target_link_directories(egel PRIVATE /usr/local/lib) # for GNU lightning
target_link_options(egel PRIVATE "-Wl,--allow-multiple-definition")
# target_link_libraries(egel stdc++fs) # for old gcc

# shared Egel library
add_library(egellib SHARED $<TARGET_OBJECTS:objlib>)
set_target_properties(egellib PROPERTIES OUTPUT_NAME egel)
target_link_libraries(egellib fmt::fmt Threads::Threads ICU::uc ICU::i18n ICU::io mimalloc-static lightning)
target_link_directories(egellib PRIVATE /usr/local/lib) # for GNU lightning

# static Egel library
add_library(egellib_static STATIC $<TARGET_OBJECTS:objlib>)
set_target_properties(egellib_static PROPERTIES OUTPUT_NAME egel)
target_link_libraries(egellib_static fmt::fmt Threads::Threads ICU::uc ICU::i18n ICU::io mimalloc-static lightning)
target_link_directories(egellib_static PRIVATE /usr/local/lib) # for GNU lightning

#
# the dynamic .ego Egel libraries

# os.ego
file(GLOB OS_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/lib/os/*.cpp")
add_library(os MODULE ${OS_LIST})
target_link_libraries(os ICU::uc ICU::i18n ICU::io mimalloc-static)
set_target_properties(os PROPERTIES PREFIX "" SUFFIX ".ego")

# fs.ego
file(GLOB FS_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/lib/fs/*.cpp")
add_library(fs MODULE ${FS_LIST})
target_link_libraries(fs ICU::uc ICU::i18n ICU::io mimalloc-static)
set_target_properties(fs PROPERTIES PREFIX "" SUFFIX ".ego")

# regex.ego
file(GLOB REGEX_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/lib/regex/*.cpp")
add_library(regex MODULE ${REGEX_LIST})
target_link_libraries(regex ICU::uc ICU::i18n ICU::io mimalloc-static)
set_target_properties(regex PROPERTIES PREFIX "" SUFFIX ".ego")

# random.ego
file(GLOB RANDOM_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/lib/random/*.cpp")
add_library(random MODULE ${RANDOM_LIST})
target_link_libraries(random ICU::uc ICU::i18n ICU::io mimalloc-static)
set_target_properties(random PROPERTIES PREFIX "" SUFFIX ".ego")

# dict.ego
file(GLOB DICT_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/lib/dict/*.cpp")
add_library(dict MODULE ${DICT_LIST})
target_link_libraries(dict ICU::uc ICU::i18n ICU::io mimalloc-static)
set_target_properties(dict PROPERTIES PREFIX "" SUFFIX ".ego")

# pqueue.ego
file(GLOB DICT_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/lib/pqueue/*.cpp")
add_library(pqueue MODULE ${DICT_LIST})
target_link_libraries(pqueue ICU::uc ICU::i18n ICU::io mimalloc-static)
set_target_properties(pqueue PROPERTIES PREFIX "" SUFFIX ".ego")

# installation
include(GNUInstallDirs)

install(TARGETS egel
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS egellib egellib_static
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${EGEL_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/egel
)

file(GLOB INCLUDE_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/*.eg")
install(FILES ${INCLUDE_LIST}
	DESTINATION  "${CMAKE_INSTALL_LIBDIR}/egel"
)

install(TARGETS os fs regex random dict pqueue
	DESTINATION  "${CMAKE_INSTALL_LIBDIR}/egel"
)

install(FILES "${CMAKE_SOURCE_DIR}/man/egel.1"
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
)
