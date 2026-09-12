echo on
cd ${{ github.workspace }} 

pwd

REM No -G here on purpose.  This used to pin "Visual Studio 17 2022", which the
REM runner image no longer ships: conan detects msvc 18 (compiler.version=195,
REM toolset v145), so cmake failed with
REM
REM   Generator "Visual Studio 17 2022" could not find any instance of Visual Studio.
REM
REM Letting cmake pick the newest installed Visual Studio keeps working when the
REM image moves again, and the conan toolchain already pins the toolset.
cmake -S %cd% ^
-B %cd%\build ^
-DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake ^
-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON ^
-DWITH_UNITTESTS=OFF ^
-DWITH_RENDERING_UNITTESTS=OFF ^
--install-prefix %cd%\installprefix

REM Fail here rather than letting a broken configure surface much later as a
REM missing CPackConfig.cmake.
if errorlevel 1 exit /b 1

rem -DWITH_UNITTESTS=OFF
cd build
dir CPack*.cmake

cmake --build . --config Release --target package
if errorlevel 1 exit /b 1

cd bin/Release
dir

WinDeployQt librecad.exe
dir ..
copy ..\*.lua
copy ..\*.json
REM Phase 5 PR-5.3 — copy path.py + lcUIPy alongside path.lua so the
REM Windows NSIS installer includes them.
copy ..\*.py 2>NUL
where makensis.exe
dir ..\..\CPack*.cmake

cpack --verbose -G NSIS --config ..\..\CPackConfig.cmake
copy LibreCAD3-*.exe ..
