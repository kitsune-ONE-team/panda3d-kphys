cmake_minimum_required(VERSION 3.10)
project(panda3d-kphys)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../../..")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    add_definitions("/wd4996 /wd4275 /wd4267 /wd4101 /wd4273 /GR-")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fno-rtti")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -fno-rtti")
endif()

if(EXISTS "${PYTHON_LIBRARY}")
else()
    find_package(PythonInterp 3.2 REQUIRED)
    find_package(PythonLibs 3.2 REQUIRED)
endif()
include_directories(${PYTHON_INCLUDE_DIR})

if(EXISTS "${BULLET_LIBRARY_DIR}")
    set(BULLET_COLLISION_LIBRARY BulletCollision)
    set(BULLET_DYNAMICS_LIBRARY BulletDynamics)
    set(BULLET_MATH_LIBRARY LinearMath)
    set(BULLET_SOFTBODY_LIBRARY BulletSoftBody)
    link_directories(${BULLET_LIBRARY_DIR})
else()
    find_package(Bullet REQUIRED)
endif()
include_directories(${BULLET_INCLUDE_DIR}/bullet)

if(${WITH_FABRIK} MATCHES "ON")
    set(IK_PATH thirdparty/ik)
    set(IK_LIB_TYPE "STATIC" CACHE STRING "SHARED or STATIC library" FORCE)
    add_subdirectory(${IK_PATH} EXCLUDE_FROM_ALL)
    include_directories(${IK_PATH}/include/public)
endif()

include_directories(${PANDA_INCLUDE_DIR}/panda3d)
link_directories(${PANDA_LIBRARY_DIR})
link_directories(${INTERROGATE_LIBRARY_DIR})

set(PANDA_LIBS "")
function(add_panda_library PATH NAME)
    if(WIN32)
        if(EXISTS "${PATH}/lib${NAME}.lib")
            list(APPEND PANDA_LIBS "lib${NAME}")
        endif()
        if(EXISTS "${PATH}/${NAME}.lib")
            list(APPEND PANDA_LIBS ${NAME})
        endif()
    else()
        list(APPEND PANDA_LIBS ${NAME})
    endif()
    set(PANDA_LIBS "${PANDA_LIBS}" PARENT_SCOPE)
endfunction()

# PRIMARY
add_panda_library(${PANDA_LIBRARY_DIR} p3framework)
# COMMON_PANDA_LIBS
add_panda_library(${PANDA_LIBRARY_DIR} panda)
add_panda_library(${PANDA_LIBRARY_DIR} pandaexpress)
# COMMON_DTOOL_LIBS
add_panda_library(${PANDA_LIBRARY_DIR} p3dtool)
add_panda_library(${PANDA_LIBRARY_DIR} p3dtoolconfig)
# EXTRA
add_panda_library(${PANDA_LIBRARY_DIR} p3direct)
add_panda_library(${INTERROGATE_LIBRARY_DIR} interrogatedb)
add_panda_library(${PANDA_LIBRARY_DIR} p3bullet)

message("Panda libraries: ${PANDA_LIBS}")
include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(kphys)

if(${WITH_TESTS} MATCHES "ON")
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kphys_tests.h.cpp
        COMMAND cxxtestgen --error-printer -o
            ${CMAKE_CURRENT_BINARY_DIR}/kphys_tests.h.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/kphys_tests.h
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kphys_tests.h)
    add_executable(kphys_tests ${CMAKE_CURRENT_BINARY_DIR}/kphys_tests.h.cpp)
    target_link_libraries(kphys_tests core)

    add_custom_command(
        OUTPUT kphys_tests__done
        COMMAND env LD_LIBRARY_PATH=${PANDA_LIBRARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/kphys_tests
        DEPENDS kphys_tests)
    add_custom_target(
        kphys_tests__run ALL
        DEPENDS kphys_tests__done)
endif()
