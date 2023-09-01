cd ${{ github.workspace }} 

pwd

cmake -S %cd% -G "Visual Studio 17 2022" ^
-B %cd%\out\build\windows-default ^
-DCMAKE_TOOLCHAIN_FILE=%cd%\conan_toolchain.cmake ^
-DCMAKE_VERBOSE_MAKEFILE=TRUE ^
-DWITH_UNITTESTS=OFF ^
-DWITH_RENDERING_UNITTESTS=OFF ^
--install-prefix %cd%\installprefix

rem -DWITH_UNITTESTS=OFF
cd out\build\windows-default

cmake --build . --config Release

cd bin

WinDeployQt librecad.exe

cd ..

cpack -C RelWithDebInfo

