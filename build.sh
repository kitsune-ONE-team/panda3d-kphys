#!/bin/sh
# build module into SDK

PYTHON_VERSION=3.13t

mkdir -p build
cd build
cmake -G "Unix Makefiles" \
    -DBULLET_INCLUDE_DIR=/var/opt/kitsunetsuki-sdk/dist/bullet/include \
    -DBULLET_LIBRARY_DIR=/var/opt/kitsunetsuki-sdk/dist/bullet/lib \
    -DCMAKE_INSTALL_PREFIX=/var/opt/kitsunetsuki-sdk/dist/sdk/lib/python${PYTHON_VERSION}/site-packages \
    -DINSTALL_PY=ON \
    -DINTERROGATE_EXECUTABLE=/var/opt/kitsunetsuki-sdk/dist/interrogate/bin/interrogate \
    -DINTERROGATE_MODULE_EXECUTABLE=/var/opt/kitsunetsuki-sdk/dist/interrogate/bin/interrogate_module \
    -DINTERROGATE_SOURCE_DIR=/var/opt/kitsunetsuki-sdk/jenkins/workspace/interrogate-lynx64 \
    -DINTERROGATE_LIBRARY_DIR=/var/opt/kitsunetsuki-sdk/dist/interrogate/lib \
    -DMULTITHREADED_BUILD=16 \
    -DPANDA_INCLUDE_DIR=/var/opt/kitsunetsuki-sdk/dist/panda3d/include \
    -DPANDA_LIBRARY_DIR=/var/opt/kitsunetsuki-sdk/dist/panda3d/lib \
    -DPYTHON_EXECUTABLE=/var/opt/kitsunetsuki-sdk/dist/python/bin/python${PYTHON_VERSION} \
    -DPYTHON_INCLUDE_DIR=/var/opt/kitsunetsuki-sdk/dist/python/include/python${PYTHON_VERSION} \
    -DPYTHON_LIBRARY=/var/opt/kitsunetsuki-sdk/dist/python/lib/libpython${PYTHON_VERSION}.so.1.0 \
    -DWITH_TESTS=ON \
    ..

make -j 16 ${*}
make install -j 16 ${*}

cd ..
