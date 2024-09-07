if(${INSTALL_PY} MATCHES "ON")
  install(
    FILES
    __init__.py
    mesh.py
    material.py
    node.py
    spring.py
    texture.py
    DESTINATION ${CMAKE_INSTALL_PREFIX}/kphys/gltf/mixins
    PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ)
endif()
