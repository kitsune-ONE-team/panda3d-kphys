if(${INSTALL_PY} MATCHES "ON")
    install(
        FILES
        __init__.py
        converter.py
        parseutils.py
        spec.py
        DESTINATION ${CMAKE_INSTALL_PREFIX}/kphys/gltf
        PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ)
endif()

add_subdirectory(mixins)
