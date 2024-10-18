#!/bin/sh
# build module into SDK

PYTHON_VERSION=3.12

mkdir -p build
cd build
cmake -G "Unix Makefiles" \
      -DBULLET_INCLUDE_DIR=/app/opt/bullet/include \
      -DBULLET_LIBRARY_DIR=/app/opt/bullet/lib \
      -DCMAKE_INSTALL_PREFIX=/app/opt/sdknt/lib/python${PYTHON_VERSION}/site-packages \
      -DINSTALL_PY=ON \
      -DINTERROGATE_EXECUTABLE=/app/opt/interrogatent/bin/interrogate \
      -DINTERROGATE_MODULE_EXECUTABLE=/app/opt/interrogatent/bin/interrogate_module \
      -DINTERROGATE_SOURCE_DIR=/app/jenkins/workspace/interrogatent-lynx64 \
      -DMULTITHREADED_BUILD=16 \
      -DPANDA_INCLUDE_DIR=/app/opt/panda3dnt/include \
      -DPANDA_LIBRARY_DIR=/app/opt/panda3dnt/lib \
      -DPYTHON_EXECUTABLE=/app/opt/pythonnt/bin/python${PYTHON_VERSION} \
      -DPYTHON_INCLUDE_DIR=/app/opt/pythonnt/include/python${PYTHON_VERSION} \
      -DPYTHON_LIBRARY=/app/opt/pythonnt/lib/libpython${PYTHON_VERSION}.so.1.0 \
      -DWITH_TESTS=ON \
      ..

make -j 16 ${*}
make install -j 16 ${*}

cd ..
