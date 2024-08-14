#!/bin/sh
# build module into SDK

PYTHON_VERSION=3.13t

mkdir -p build
cd build
cmake -G "Unix Makefiles" \
    -DBULLET_INCLUDE_DIR=/app/opt/bullet/include \
    -DBULLET_LIBRARY_DIR=/app/opt/bullet/lib \
    -DCMAKE_INSTALL_PREFIX=/app/opt/sdk/lib/python${PYTHON_VERSION}/site-packages \
    -DINSTALL_PY=ON \
    -DINTERROGATE_EXECUTABLE=/app/opt/interrogate/bin/interrogate \
    -DINTERROGATE_MODULE_EXECUTABLE=/app/opt/interrogate/bin/interrogate_module \
    -DINTERROGATE_SOURCE_DIR=/app/jenkins/workspace/interrogate-lynx64 \
    -DMULTITHREADED_BUILD=16 \
    -DPANDA_INCLUDE_DIR=/app/opt/panda3d/include \
    -DPANDA_LIBRARY_DIR=/app/opt/panda3d/lib \
    -DPYTHON_EXECUTABLE=/app/opt/python/bin/python${PYTHON_VERSION} \
    -DPYTHON_INCLUDE_DIR=/app/opt/python/include/python${PYTHON_VERSION} \
    -DPYTHON_LIBRARY=/app/opt/python/lib/libpython${PYTHON_VERSION}.so.1.0 \
    -DWITH_TESTS=ON \
    ..

make -j 16 ${*}
make install -j 16 ${*}

cd ..
