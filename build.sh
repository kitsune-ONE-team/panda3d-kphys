#!/bin/sh

mkdir -p build
cd build
cmake -G "Unix Makefiles" \
    -DCMAKE_INSTALL_PREFIX=/panda3d-kphys/lib/python3.11/site-packages \
    -DBULLET_INCLUDE_DIR=/root/jenkins/workspace/bullet-lynx64/dist/bullet/include \
    -DBULLET_LIBRARY_DIR=/root/jenkins/workspace/bullet-lynx64/dist/bullet/lib \
    -DINSTALL_PY=ON \
    -DWITH_TESTS=ON \
    -DPANDA_BINARY_DIR=/root/jenkins/workspace/panda3d-lynx64/dist/panda3d/bin \
    -DPANDA_INCLUDE_DIR=/root/jenkins/workspace/panda3d-lynx64/dist/panda3d/include \
    -DPANDA_LIBRARY_DIR=/root/jenkins/workspace/panda3d-lynx64/dist/panda3d/lib \
    -DPYTHON_INCLUDE_DIR=/root/jenkins/workspace/python-lynx64/dist/python/include/python3.11 \
    -DPYTHON_EXECUTABLE=/root/jenkins/workspace/python-lynx64/dist/python/bin/python3.11 \
    -DPYTHON_LIBRARY=/root/jenkins/workspace/python-lynx64/dist/python/lib/libpython3.11.so.1.0 \
    ..

make
make install DESTDIR=../dist/

cd ..

export PYTHONPATH=\
/root/jenkins/workspace/panda3d-lynx64/dist/panda3d/lib/python3.11/site-packages:\
dist/panda3d-kphys/lib/python3.11/site-packages

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/root/jenkins/workspace/panda3d-lynx64/dist/panda3d/lib

/root/jenkins/workspace/python-lynx64/dist/python/bin/python3.11 samples/animation.py
