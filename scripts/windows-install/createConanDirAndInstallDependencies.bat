echo "conan"
for %%i in ("%~dp0..\..") do set "LIBRECAD_ROOT=%%~fi"
cd /d "%LIBRECAD_ROOT%"
if errorlevel 1 exit /b 1
if not exist conan mkdir conan
if errorlevel 1 exit /b 1
pushd conan
if errorlevel 1 exit /b 1
conan profile detect --force
if errorlevel 1 (popd & exit /b 1)

REM Use Conan Center's prebuilt Windows 2022/MSVC binaries only. The workflow
REM pins windows-2022 to match Qt's MSVC 2022 package ABI; fail immediately if
REM any dependency has no compatible binary instead of compiling it from source.
REM Keep dependency package IDs on the published C++14 binaries; the LibreCAD
REM CMake project independently selects C++17 for its own sources.
conan install .. -s build_type=Release -s compiler.cppstd=14 --build=never --output-folder "%LIBRECAD_ROOT%\conan"
if errorlevel 1 (popd & exit /b 1)
popd
if not exist conan\conan_toolchain.cmake exit /b 1
if not exist conan\conanrun.bat exit /b 1
