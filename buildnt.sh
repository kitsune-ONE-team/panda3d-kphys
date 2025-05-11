#!/bin/sh
SDK=/var/opt/kitsunetsuki-sdk
PYTHON_VERSION=3.12

mkdir -p build
cd build
cmake -G "Unix Makefiles" \
    -DBULLET_INCLUDE_DIR=${SDK}/dist/bullet/include \
    -DBULLET_LIBRARY_DIR=${SDK}/dist/bullet/lib \
    -DCMAKE_INSTALL_PREFIX=../dist/sdknt/lib/python${PYTHON_VERSION}/site-packages \
    -DINSTALL_PY=ON \
    -DINTERROGATE_EXECUTABLE=${SDK}/dist/interrogatent/bin/interrogate \
    -DINTERROGATE_MODULE_EXECUTABLE=${SDK}/dist/interrogatent/bin/interrogate_module \
    -DINTERROGATE_SOURCE_DIR=${SDK}/jenkins/workspace/interrogate-lynx64 \
    -DINTERROGATE_LIBRARY_DIR=${SDK}/dist/interrogatent /lib \
    -DMULTITHREADED_BUILD=16 \
    -DPANDA_INCLUDE_DIR=${SDK}/dist/panda3dnt/include \
    -DPANDA_LIBRARY_DIR=${SDK}/dist/panda3dnt/lib \
    -DPYTHON_EXECUTABLE=${SDK}/dist/pythonnt/bin/python${PYTHON_VERSION} \
    -DPYTHON_INCLUDE_DIR=${SDK}/dist/pythonnt/include/python${PYTHON_VERSION} \
    -DPYTHON_LIBRARY=${SDK}/dist/pythonnt/lib/libpython${PYTHON_VERSION}.so.1.0 \
    -DWITH_TESTS=ON \
    ..

make -j 16 ${*}
make install -j 16 ${*}

cd ..
