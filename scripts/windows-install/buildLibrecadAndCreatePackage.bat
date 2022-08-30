cd ${{ github.workspace }} 

cmake -S %cd% -B %cd%\out\build\windows-default -DCMAKE_BUILD_TYPE=Debug --install-prefix %cd%\installprefix

cmake -S %cd% -B %cd%\out\build\windows-default -DCMAKE_BUILD_TYPE=Debug --install-prefix %cd%\installprefix

cd out\build\windows-default

cmake --build . --config Debug

cd bin

WinDeployQt librecad.exe

cd ..

cpack -C Debug

