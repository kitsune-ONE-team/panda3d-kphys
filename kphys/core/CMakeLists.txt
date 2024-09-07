set(CORE_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/bullet/controller.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/animation.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/animator.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/armature.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/bone.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/bvhq.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/ccdik.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/channel.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/config.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/controller.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/converters.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/effector.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/frame.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/hit.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/hitbox.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/ik.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/multianimation.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/multianimator.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/puppet.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/puppetmaster.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/spring.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/spring2.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/types.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/wigglebone.cxx
)

set(CORE_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/bullet/controller.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/animation.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/animator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/armature.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/bone.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/bvhq.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/ccdik.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/channel.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/config.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/controller.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/converters.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/effector.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/frame.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/hit.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/hitbox.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/ik.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/multianimation.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/multianimator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/puppet.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/puppetmaster.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/spring.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/spring2.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/panda/wigglebone.h
)

set(CORE_LIBS
  # PANDA3D
  ${PANDA_LIBS}
  # PYTHON
  ${PYTHON_LIBRARY}
  # BULLET
  ${BULLET_SOFTBODY_LIBRARY}
  ${BULLET_DYNAMICS_LIBRARY}
  ${BULLET_COLLISION_LIBRARY}
  ${BULLET_MATH_LIBRARY}
)

if(${WITH_FABRIK} MATCHES "ON")
  set(CORE_LIBS ${CORE_LIBS} ik)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(
  OUTPUT core.in core_igate.cpp
  COMMAND ${INTERROGATE_EXECUTABLE} -DCPPPARSER -D__STDC__=1 -D__cplusplus=201103L
  -I${CMAKE_SOURCE_DIR}
  -I${PANDA_INCLUDE_DIR}/panda3d
  -S${CMAKE_SOURCE_DIR}
  -S${INTERROGATE_SOURCE_DIR}/parser-inc
  -oc core_igate.cpp
  -od core.in
  -fnames -string -refcount -assert -python-native -nomangle
  -module core -library core
  ${CORE_HEADERS}
  DEPENDS ${CORE_SOURCES} ${CORE_HEADERS})

add_custom_command(
  OUTPUT core_module.cpp
  COMMAND ${INTERROGATE_MODULE_EXECUTABLE}
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

target_link_libraries(core ${CORE_LIBS})

install(TARGETS core DESTINATION ${CMAKE_INSTALL_PREFIX}/kphys)
