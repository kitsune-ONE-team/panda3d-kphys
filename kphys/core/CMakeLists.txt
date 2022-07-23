set(CORE_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/bullet/controller.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/armature.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/bone.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/config.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/controller.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/converters.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/effector.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/hitbox.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/ik.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/spring.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/types.cxx
)

set(CORE_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/bullet/controller.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/armature.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/bone.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/controller.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/converters.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/effector.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/hitbox.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/ik.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/spring.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/types.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(
  OUTPUT core.in core_igate.cpp
  COMMAND interrogate -DCPPPARSER -D__STDC__=1 -D__cplusplus=201103L
  -I${CMAKE_CURRENT_SOURCE_DIR}
  -I${PANDA_INCLUDE_DIR}/panda3d
  -S${CMAKE_CURRENT_SOURCE_DIR}
  -S${PANDA_INCLUDE_DIR}/panda3d/parser-inc
  -oc core_igate.cpp
  -od core.in
  -fnames -string -refcount -assert -python-native -nomangle
  -module core -library core
  ${CORE_HEADERS}
  DEPENDS ${CORE_SOURCES} ${CORE_HEADERS})

add_custom_command(
  OUTPUT core_module.cpp
  COMMAND interrogate_module
  -oc core_module.cpp
  -module core -library core
  -python-native
  -import panda3d.core
  -import panda3d.bullet
  core.in
  DEPENDS core.in core_igate.cpp)

add_library(core SHARED
  ${CORE_SOURCES}
  core_igate.cpp
  core_module.cpp)

set_target_properties(core PROPERTIES PREFIX "")
if(WIN32)
  set_target_properties(core PROPERTIES SUFFIX ".pyd")
endif()

target_link_libraries(core
  # PANDA3D
  ${PANDA_LIBS}
  # PYTHON
  ${PYTHON_LIBRARY}
  # BULLET
  ${BULLET_COLLISION_LIBRARY}
  ${BULLET_DYNAMICS_LIBRARY}
  ${BULLET_MATH_LIBRARY}
  ${BULLET_SOFTBODY_LIBRARY}
  # IK
  ik
)

install(TARGETS core DESTINATION ${CMAKE_INSTALL_PREFIX}/kphys)
