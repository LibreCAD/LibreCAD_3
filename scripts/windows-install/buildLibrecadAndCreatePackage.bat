cd ${{ github.workspace }} 

cmake -S %cd% -B %cd%\out\build\windows-default -DCMAKE_BUILD_TYPE=RelWithDebInfo --install-prefix %cd%\installprefix

cmake -S %cd% -B %cd%\out\build\windows-default -DCMAKE_BUILD_TYPE=RelWithDebInfo --install-prefix %cd%\installprefix

cd out\build\windows-default

cmake --build . --config RelWithDebInfo

cd bin

WinDeployQt librecad.exe

cd ..

cpack -C RelWithDebInfo

