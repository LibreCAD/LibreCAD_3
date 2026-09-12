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

conan install .. -s build_type=Release --build missing
REM Without this the step reported success even when conan had failed, and the
REM build only broke later, somewhere unrelated.
if errorlevel 1 (popd & exit /b 1)
popd
