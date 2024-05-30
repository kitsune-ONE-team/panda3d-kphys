call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" amd64 10.0.20348.0
@echo on

set JENKINS_WS=%USERPROFILE%\Jenkins\Jenkins\workspace

if not exist build (
    mkdir build
)
cd build
cmake -G "NMake Makefiles" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX=\sdk\lib\site-packages ^
    -DBULLET_INCLUDE_DIR=%JENKINS_WS%\bullet-winx64\dist\bullet\include ^
    -DBULLET_LIBRARY_DIR=%JENKINS_WS%\bullet-winx64\dist\bullet\lib ^
    -DINSTALL_PY=ON ^
    -DPANDA_BINARY_DIR=%JENKINS_WS%\panda3d-winx64\dist\panda3d\bin ^
    -DPANDA_INCLUDE_DIR=%JENKINS_WS%\panda3d-winx64\dist\panda3d\include ^
    -DPANDA_LIBRARY_DIR=%JENKINS_WS%\panda3d-winx64\dist\panda3d\lib ^
    -DPYTHON_INCLUDE_DIR=%JENKINS_WS%\python-winx64\dist\python\include ^
    -DPYTHON_EXECUTABLE=%JENKINS_WS%\python-winx64\dist\python\bin\python ^
    -DPYTHON_LIBRARY=%JENKINS_WS%\python-winx64\dist\python\libs\python311.lib ^
    ..

if "%ERRORLEVEL%" == "1" (
    exit /B 1
)

nmake
rem nmake install DESTDIR=..\dist\
nmake install DESTDIR=%JENKINS_WS%\sdk-winx64\dist\
cd ..

if "%ERRORLEVEL%" == "1" (
    exit /B 1
)

%JENKINS_WS%\sdk-winx64\dist\sdk\bin\python samples\animation.py
