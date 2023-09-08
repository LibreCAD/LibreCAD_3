echo on
cd ${{ github.workspace }} 

pwd

cmake -S %cd% -G "Visual Studio 17 2022" ^
-B %cd%\build ^
-DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake ^
-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON ^
-DWITH_UNITTESTS=OFF ^
-DWITH_RENDERING_UNITTESTS=OFF ^
--install-prefix %cd%\installprefix

rem -DWITH_UNITTESTS=OFF
cd build
dir CPack*.cmake

cmake --build . --config Release --target package

cd bin/Release
dir

WinDeployQt librecad.exe
dir ..
copy ..\*.lua ..\*.json .
where makensis.exe
dir ..\..\CPack*.cmake

cpack --verbose -G NSIS --config ..\..\CPackConfig.cmake
copy LibreCAD3-*.exe ..
