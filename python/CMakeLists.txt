# minimal cmake version
cmake_minimum_required(VERSION 3.13)

# fallback for other versions 
if(${CMAKE_VERSION} VERSION_LESS 3.18)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.18)
endif()

# set the cxx compiler standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# steal the version of the bridge XXX: why the hell did this stop working?
set(EGEL_MAJOR_REGEX "#define LIBRARY_EGEL_MAJOR[ \t]+\"(.+)\"")
set(EGEL_MINOR_REGEX "#define LIBRARY_EGEL_MINOR[ \t]+\"(.+)\"")
set(EGEL_PATCH_REGEX "#define LIBRARY_EGEL_PATCH[ \t]+\"(.+)\"")

file(STRINGS "${CMAKE_SOURCE_DIR}/src/python.cpp"
    EGEL_MAJOR REGEX ${EGEL_MAJOR_REGEX})
file(STRINGS "${CMAKE_SOURCE_DIR}/src/python.cpp"
    EGEL_MINOR REGEX ${EGEL_MINOR_REGEX})
file(STRINGS "${CMAKE_SOURCE_DIR}/src/python.cpp"
    EGEL_PATCH REGEX ${EGEL_PATCH_REGEX})

string(REGEX REPLACE ${EGEL_MAJOR_REGEX} "\\1" EGEL_MAJOR "${EGEL_MAJOR}")
string(REGEX REPLACE ${EGEL_MINOR_REGEX} "\\1" EGEL_MINOR "${EGEL_MINOR}")
string(REGEX REPLACE ${EGEL_PATCH_REGEX} "\\1" EGEL_PATCH "${EGEL_PATCH}")

#set(PVERSION ${EGEL_MAJOR}.${EGEL_MINOR}.${EGEL_PATCH})
set(PVERSION 0.0.1)

message("python bridge for egel ${PVERSION}")

# add 'fallback' modules
set(CMAKE_MODULE_PATH "../contrib/cmake/modules;${CMAKE_MODULE_PATH}")

# the Python/Egel bridge
project(Python DESCRIPTION "python bridge for egel"
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
#find_package(PythonLibs 3.7 REQUIRED)
find_package (Python3 3.10 REQUIRED COMPONENTS Interpreter Development)

include_directories(${Python3_INCLUDE_DIR})
include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories(/usr/local/include)

# python.ego
file(GLOB PYTHON_EGO_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_library(python_ego MODULE ${PYTHON_EGO_LIST})
target_link_libraries(python_ego ICU::uc ICU::i18n ICU::io)
target_link_libraries(python_ego ${PYTHON_LIBRARIES})
set_target_properties(python_ego PROPERTIES PREFIX "" SUFFIX ".ego")

# installation
include(GNUInstallDirs)

install(TARGETS python_ego
	DESTINATION  "${CMAKE_INSTALL_LIBDIR}/egel"
)

