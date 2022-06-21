setx QTDIR "%cd%\Qt\5.15.2\msvc2019_64" & setx QT_QPA_PLATFORM_PLUGIN_PATH "%QTDIR%\plugins\platforms\"

Powershell.exe -Command "[Environment]::SetEnvironmentVariable('Path', $env:Path + ';%cd%\Qt\5.15.2\msvc2019_64;%cd%\LibreCAD_3\out\build\x64-Debug\lib;%cd%\Qt\5.15.2\msvc2019_64\bin', 'User')"