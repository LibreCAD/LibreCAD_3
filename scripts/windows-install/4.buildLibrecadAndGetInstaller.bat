cd LibreCAD_3

cmake -S C:\Users\CRiSTiK\Desktop\git\scriptsLibrecad3\LibreCAD_3 -B C:\Users\CRiSTiK\Desktop\git\scriptsLibrecad3\LibreCAD_3\out\build\windows-default -DCMAKE_BUILD_TYPE=RelWithDebInfo --install-prefix C:\Users\CRiSTiK\Desktop\git\scriptsLibrecad3\LibreCAD_3\installprefix

cmake -S C:\Users\CRiSTiK\Desktop\git\scriptsLibrecad3\LibreCAD_3 -B C:\Users\CRiSTiK\Desktop\git\scriptsLibrecad3\LibreCAD_3\out\build\windows-default -DCMAKE_BUILD_TYPE=RelWithDebInfo --install-prefix C:\Users\CRiSTiK\Desktop\git\scriptsLibrecad3\LibreCAD_3\installprefix

cd out\build\windows-default

cmake --build . --config RelWithDebInfo

cd bin\RelWithDebInfo

WinDeployQt librecad.exe

cd ..\..

cpack -C RelWithDebInfo


_____________________________________

