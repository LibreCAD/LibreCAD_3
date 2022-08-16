cd ${{ github.workspace }} 

cmake -S ${{ github.workspace }} -B ${{ github.workspace }}\out\build\windows-default -DCMAKE_BUILD_TYPE=RelWithDebInfo --install-prefix ${{ github.workspace }}\installprefix

cmake -S ${{ github.workspace }} -B ${{ github.workspace }}\out\build\windows-default -DCMAKE_BUILD_TYPE=RelWithDebInfo --install-prefix ${{ github.workspace }}\installprefix

cd out\build\windows-default

cmake --build . --config RelWithDebInfo

cd bin\RelWithDebInfo

WinDeployQt librecad.exe

cd ..\..

cpack -C RelWithDebInfo

