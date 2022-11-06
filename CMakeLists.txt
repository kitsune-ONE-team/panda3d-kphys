cmake_minimum_required(VERSION 3.10)
project(panda3d-kphys)

if(WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_definitions("/wd4996 /wd4275 /wd4267 /wd4101 /wd4273")
else()
  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11")
endif()

find_package(PythonInterp 3.2 REQUIRED)
find_package(PythonLibs 3.2 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIR})

find_package(Bullet REQUIRED)
include_directories(${BULLET_INCLUDE_DIR})

set(IK_PATH thirdparty/ik)
set(IK_LIB_TYPE "STATIC" CACHE STRING "SHARED or STATIC library" FORCE)
add_subdirectory(${IK_PATH} EXCLUDE_FROM_ALL)
include_directories(${IK_PATH}/include/public)

find_path(PANDA_INCLUDE_DIR NAMES panda3d/pandaSystem.h)
include_directories(${PANDA_INCLUDE_DIR}/panda3d)
find_library(PANDAFRAMEWORK_LIBRARY_PATH NAMES libp3framework.so libp3framework.lib)
get_filename_component(PANDA_LIBRARY_DIR ${PANDAFRAMEWORK_LIBRARY_PATH} DIRECTORY)
link_directories(${PANDA_LIBRARY_DIR})

if(WIN32)
  set(PANDA_LIBS
    # PRIMARY
    libp3framework
    # COMMON_PANDA_LIBS
    libpanda
    libpandaexpress
    # COMMON_DTOOL_LIBS
    libp3dtool
    libp3dtoolconfig
    # EXTRA
    libp3direct
    libp3interrogatedb
    # PANDABULLET
    libp3bullet
    )
else()
  set(PANDA_LIBS
    # PRIMARY
    p3framework
    # COMMON_PANDA_LIBS
    panda
    pandaexpress
    # COMMON_DTOOL_LIBS
    p3dtool
    p3dtoolconfig
    # EXTRA
    p3direct
    p3interrogatedb
    # PANDABULLET
    p3bullet
    )
endif()

include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(kphys)
