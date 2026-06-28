#!/bin/sh
SDK=/var/opt/ksdk
PYTHON_VERSION=3.14t

mkdir -p build
cd build
cmake -G "Unix Makefiles" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DBULLET_INCLUDE_DIR=${SDK}/dist/ksdk/include \
    -DBULLET_LIBRARY_DIR=${SDK}/dist/ksdk//lib \
    -DCMAKE_INSTALL_PREFIX=../dist/sdk/lib/python${PYTHON_VERSION}/site-packages \
    -DINSTALL_PY=ON \
    -DINTERROGATE_EXECUTABLE=${SDK}/dist/interrogate/bin/interrogate \
    -DINTERROGATE_MODULE_EXECUTABLE=${SDK}/dist/interrogate/bin/interrogate_module \
    -DINTERROGATE_SOURCE_DIR=${SDK}/source/interrogate \
    -DINTERROGATE_LIBRARY_DIR=${SDK}/dist/interrogate/lib \
    -DMULTITHREADED_BUILD=16 \
    -DPANDA_INCLUDE_DIR=${SDK}/dist/ksdk/include \
    -DPANDA_LIBRARY_DIR=${SDK}/dist/ksdk/lib \
    -DPYTHON_EXECUTABLE=${SDK}/dist/ksdk/bin/python${PYTHON_VERSION} \
    -DPYTHON_INCLUDE_DIR=${SDK}/dist/ksdk/include/python${PYTHON_VERSION} \
    -DPYTHON_LIBRARY=${SDK}/dist/ksdk/lib/libpython${PYTHON_VERSION}.so.1.0 \
    -DWITH_TESTS=ON \
    ..

make -j 16 ${*}
make install -j 16 ${*}

cd ..
