echo "conan"
pwd
mkdir conan
pushd conan
conan profile detect --force
if errorlevel 1 (popd & exit /b 1)

REM CMake 4 removed compatibility with cmake_minimum_required(VERSION <3.5),
REM and conan builds libdxfrw/2.2.0 from source, whose CMakeLists still asks
REM for an older minimum.  The configure then fails with
REM
REM   Compatibility with CMake < 3.5 has been removed from CMake.
REM
REM taking conan_toolchain.cmake with it, so the LibreCAD build that follows
REM failed confusingly on a missing toolchain file rather than on the real
REM cause.  CMake offers this variable as the supported escape hatch and
REM honours it from the environment, so every package conan configures picks
REM it up.  Drop this once the dependencies declare a >= 3.5 minimum.
set CMAKE_POLICY_VERSION_MINIMUM=3.5
for %%i in ("%cd%\..") do set LIBRECAD_ROOT=%%~fi

REM Build the same patched libdxfrw revision used by the Unix providers.  The
REM ConanCenter recipe has a different source revision and still drops 102
REM custom-entity app-data groups on read.
set LIBDXFRW_REVISION=f7debf4c5f30734a70c5e828cf05247aabc2d04b
set LIBDXFRW_SOURCE=%LIBRECAD_ROOT%\build\libdxfrw-source
set LIBDXFRW_PREFIX=%LIBRECAD_ROOT%\build\dependencies\libdxfrw
if not exist "%LIBDXFRW_SOURCE%\.git" (
    git clone --branch LibreCAD_3 https://github.com/LibreCAD/libdxfrw "%LIBDXFRW_SOURCE%"
    if errorlevel 1 (popd & exit /b 1)
    git -C "%LIBDXFRW_SOURCE%" checkout --detach %LIBDXFRW_REVISION%
    if errorlevel 1 (popd & exit /b 1)
)
for /f %%i in ('git -C "%LIBDXFRW_SOURCE%" rev-parse HEAD') do set LIBDXFRW_ACTUAL_REVISION=%%i
if not "%LIBDXFRW_ACTUAL_REVISION%"=="%LIBDXFRW_REVISION%" (
    echo libdxfrw revision mismatch: expected %LIBDXFRW_REVISION%, got %LIBDXFRW_ACTUAL_REVISION%
    popd & exit /b 1
)
bash "%LIBRECAD_ROOT%\scripts\patch-libdxfrw.sh" "%LIBDXFRW_SOURCE%"
if errorlevel 1 (popd & exit /b 1)
cmake -S "%LIBDXFRW_SOURCE%" -B "%LIBDXFRW_SOURCE%\build" ^
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ^
    -DCMAKE_INSTALL_PREFIX="%LIBDXFRW_PREFIX%"
if errorlevel 1 (popd & exit /b 1)
cmake --build "%LIBDXFRW_SOURCE%\build" --config Release
if errorlevel 1 (popd & exit /b 1)
cmake --install "%LIBDXFRW_SOURCE%\build" --config Release
if errorlevel 1 (popd & exit /b 1)

conan install .. -s build_type=Release --build missing
REM Without this the step reported success even when conan had failed, and the
REM build only broke later, somewhere unrelated.
if errorlevel 1 (popd & exit /b 1)
popd
