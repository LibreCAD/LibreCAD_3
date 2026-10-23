echo on
cd ${{ github.workspace }} 

pwd

if not defined Python_ROOT_DIR (
    echo Python_ROOT_DIR was not set by the Windows CI SDK setup
    exit /b 1
)
if not defined Python_EXECUTABLE (
    echo Python_EXECUTABLE was not set by the Windows CI SDK setup
    exit /b 1
)

REM No -G here on purpose.  This used to pin "Visual Studio 17 2022", which the
REM runner image no longer ships: conan detects msvc 18 (compiler.version=195,
REM toolset v145), so cmake failed with
REM
REM   Generator "Visual Studio 17 2022" could not find any instance of Visual Studio.
REM
REM Letting cmake pick the newest installed Visual Studio keeps working when the
REM image moves again, and the conan toolchain already pins the toolset.
cmake -S %cd% ^
-B %cd%\build ^
-DCMAKE_TOOLCHAIN_FILE="%cd%\conan\conan_toolchain.cmake" ^
-DPython_ROOT_DIR="%Python_ROOT_DIR%" ^
-DPython_EXECUTABLE="%Python_EXECUTABLE%" ^
-DWITH_PYTHONSCRIPT=ON ^
-DWITH_UNITTESTS=ON ^
-DWITH_LUACMDINTERFACE=OFF ^
-Dlibdxfrw_DIR="%cd%\build\dependencies\libdxfrw\lib\cmake\libdxfrw" ^
-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON ^
-DWITH_RENDERING_UNITTESTS=OFF ^
--install-prefix %cd%\installprefix

REM Fail here rather than letting a broken configure surface much later as a
REM missing CPackConfig.cmake.
if errorlevel 1 exit /b 1

cd build
call ..\conan\conanrun.bat
if errorlevel 1 exit /b 1

cmake --build . --config Release
if errorlevel 1 exit /b 1

REM CMakeLists.txt:10-14 pins CMAKE_RUNTIME_OUTPUT_DIRECTORY(_<CONFIG>) to
REM build/bin for every configuration (avoiding a bin/Release,
REM bin/RelWithDebInfo, ... split), so librecad.exe and its path.lua/
REM *.json companions all land directly in build/bin -- there is
REM no build/bin/Release. `cd bin/Release` used to fail silently (bare
REM `cd` doesn't stop the script), leaving every command below it
REM operating one directory higher than intended: WinDeployQt ran
REM against a librecad.exe that "does not exist", and cpack's --config
REM path resolved two levels above the actual CPackConfig.cmake.
cd bin
dir

if not exist lcunittest.exe (
    echo Python-enabled unit test executable was not built
    exit /b 1
)
set QT_QPA_PLATFORM=offscreen
lcunittest.exe --gtest_list_tests > ..\python-tests.txt
if errorlevel 1 exit /b 1
findstr /C:"PythonRuntime." ..\python-tests.txt > nul
if errorlevel 1 (
    echo Python runtime tests were not discovered
    exit /b 1
)
findstr /C:"PythonFixture." ..\python-tests.txt > nul
if errorlevel 1 (
    echo Python binding fixture tests were not discovered
    exit /b 1
)
"%Python_EXECUTABLE%" ..\..\scripts\run_required_gtests.py --binary lcunittest.exe --filter="PythonRuntime.*:PythonFixture.*"
if errorlevel 1 exit /b 1

REM Keep Python DXF/DXB serialization compatibility blocking on Windows. The
REM matrix exercises all advertised versions and the pinned libdxfrw binary
REM writer patch used by this package.
findstr /C:"PersistenceTest." ..\python-tests.txt > nul
if errorlevel 1 (
    echo Python persistence tests were not discovered
    exit /b 1
)
"%Python_EXECUTABLE%" ..\..\scripts\run_required_gtests.py --binary lcunittest.exe --filter="PersistenceTest.*"
if errorlevel 1 exit /b 1

REM Keep the equivalent Lua/Python geometry and undo/redo comparison blocking
REM on Windows. It runs separately so its interpreter instances do not share
REM lifecycle state with the runtime/binding fixture process.
findstr /C:"LuaRuntimeTest." ..\python-tests.txt > nul
if errorlevel 1 (
    echo Lua runtime regression tests were not discovered
    exit /b 1
)
findstr /C:"ParityTest." ..\python-tests.txt > nul
if errorlevel 1 (
    echo Lua/Python parity tests were not discovered
    exit /b 1
)
"%Python_EXECUTABLE%" ..\..\scripts\run_required_gtests.py --binary lcunittest.exe --filter="LuaRuntimeTest.*:ParityTest.*"
if errorlevel 1 exit /b 1

REM Keep scripting lifecycle, conversion, custom-entity, GUI bridge, and
REM ScriptDock coverage blocking on Windows. Guard each suite so another
REM matching suite cannot hide a missing compiled test target.
for %%S in (EventBus. ScriptValueRoundTrip. ScriptCallbackInvoke. ScriptCallbackEquality. ScriptCallbackLifecycle. CustomEntityDxfRoundTrip. ScriptCustomEntity. ScriptCustomEntityBuilder. ScriptCustomEntityLuaRegistration. MWindowTest. PyGuiBridgeTest. ScriptDockTest.) do (
    findstr /C:"%%S" ..\python-tests.txt > nul
    if errorlevel 1 (
        echo Required scripting/UI test suite was not discovered: %%S
        exit /b 1
    )
)
findstr /C:"PythonCreatesLine" ..\python-tests.txt > nul
if errorlevel 1 (
    echo Python ScriptDock creation test was not discovered
    exit /b 1
)
findstr /C:"PythonEventDrivenOnEventReachesMainWindow" ..\python-tests.txt > nul
if errorlevel 1 (
    echo Python ScriptDock event test was not discovered
    exit /b 1
)
"%Python_EXECUTABLE%" ..\..\scripts\run_required_gtests.py --binary lcunittest.exe --filter="EventBus.*:ScriptValueRoundTrip.*:ScriptCallbackInvoke.*:ScriptCallbackEquality.*:ScriptCallbackLifecycle.*:CustomEntityDxfRoundTrip.*:ScriptCustomEntity.*:ScriptCustomEntityBuilder.*:ScriptCustomEntityLuaRegistration.*:MWindowTest.*:PyGuiBridgeTest.*:ScriptDockTest.Python*"
if errorlevel 1 exit /b 1

WinDeployQt librecad.exe
if errorlevel 1 exit /b 1
dir ..
where makensis.exe
dir ..\CPack*.cmake

cd ..
cpack --verbose -G NSIS --config CPackConfig.cmake
if errorlevel 1 exit /b 1
exit /b 0
