echo on
for %%i in ("%~dp0..\..") do set "LIBRECAD_ROOT=%%~fi"
cd /d "%LIBRECAD_ROOT%"
if errorlevel 1 exit /b 1

if not defined Python_ROOT_DIR (
    echo Python_ROOT_DIR was not set by the Windows CI SDK setup
    exit /b 1
)
if not defined Python_EXECUTABLE (
    echo Python_EXECUTABLE was not set by the Windows CI SDK setup
    exit /b 1
)
"%Python_EXECUTABLE%" -m unittest scripts.tests.test_validate_python_provider -v
if errorlevel 1 exit /b 1
"%Python_EXECUTABLE%" -m unittest scripts.tests.test_windows_package_smoke -v
if errorlevel 1 exit /b 1

REM No -G here on purpose. Both Windows jobs pin windows-2022, whose installed
REM Visual Studio 2022 matches Qt's win64_msvc2022_64 package and Conan's
REM prebuilt MSVC ABI. Keep CMake's default generator consistent with the
REM persistence-only job so both exercise the same compiler/toolset.
cmake -S %cd% ^
-B %cd%\build ^
-DCMAKE_TOOLCHAIN_FILE="%cd%\conan\conan_toolchain.cmake" ^
-DPython_ROOT_DIR="%Python_ROOT_DIR%" ^
-DPython_EXECUTABLE="%Python_EXECUTABLE%" ^
-DWITH_PYTHONSCRIPT=ON ^
-DWITH_UNITTESTS=ON ^
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
"%Python_EXECUTABLE%" ..\..\scripts\run_required_gtests.py --binary lcunittest.exe --filter="PythonRuntime.*:PythonFixture.*" --require-suite "PythonRuntime." --require-suite "PythonFixture."
if errorlevel 1 exit /b 1

REM Keep the equivalent Lua/Python geometry and undo/redo comparison blocking
REM on Windows. It runs separately so its interpreter instances do not share
REM lifecycle state with the runtime/binding fixture process.
"%Python_EXECUTABLE%" ..\..\scripts\run_required_gtests.py --binary lcunittest.exe --filter="LuaRuntimeTest.*:ParityTest.*" --require-suite "LuaRuntimeTest." --require-suite "ParityTest."
if errorlevel 1 exit /b 1

REM Keep the Qt thread-affinity regression blocking on Windows too. Run it in
REM its own process so QApplication/Python lifecycle state cannot leak from the
REM core Python fixture process above.
"%Python_EXECUTABLE%" ..\..\scripts\run_required_gtests.py --binary lcunittest.exe --filter="MWindowTest.PyGuiBridgeRejectsWorkerThreadAccess" --require-suite "MWindowTest."
if errorlevel 1 exit /b 1

REM Run the cross-cutting Python/runtime/UI/DXF/Lua regression set together as
REM well. The macOS arm64 reference run passes this exact 21-suite filter;
REM keeping it in the Windows package lane makes API and lifecycle regressions
REM blocking before WinDeployQt and NSIS packaging.
"%Python_EXECUTABLE%" ..\..\scripts\run_required_gtests.py --binary lcunittest.exe --filter="PythonFixture.*:PythonRuntime.*:ParityTest.*:ScriptValueRoundTrip.*:ScriptCallbackInvoke.*:ScriptCallbackEquality.*:ScriptCallbackLifecycle.*:ScriptObject.*:EventBus.*:LuaCustomEntityManager.*:ScriptCustomEntity.*:ScriptCustomEntityBuilder.*:CustomEntityDxfRoundTrip.*:ScriptCustomEntityLuaRegistration.*:DwgImportTest.*:PersistenceTest.*:ScriptDockTest.*:MWindowTest.*:ToolbarButtonTest.*:PyGuiBridgeTest.*:MenuEntriesTest.*" --require-suite "PythonFixture." --require-suite "PythonRuntime." --require-suite "ParityTest." --require-suite "ScriptValueRoundTrip." --require-suite "ScriptCallbackInvoke." --require-suite "ScriptCallbackEquality." --require-suite "ScriptCallbackLifecycle." --require-suite "ScriptObject." --require-suite "EventBus." --require-suite "LuaCustomEntityManager." --require-suite "ScriptCustomEntity." --require-suite "ScriptCustomEntityBuilder." --require-suite "CustomEntityDxfRoundTrip." --require-suite "ScriptCustomEntityLuaRegistration." --require-suite "DwgImportTest." --require-suite "PersistenceTest." --require-suite "ScriptDockTest." --require-suite "MWindowTest." --require-suite "ToolbarButtonTest." --require-suite "PyGuiBridgeTest." --require-suite "MenuEntriesTest."
if errorlevel 1 exit /b 1

WinDeployQt --include-plugins qoffscreen librecad.exe
if errorlevel 1 exit /b 1

REM Validate the install tree separately from the build tree before creating
REM the NSIS package. Poison Python settings, run outside the checkout, and
REM restrict PATH to the staged app plus Windows system directories so an
REM unrelated Python/Conan/Qt installation cannot rescue a broken runtime.
cd /d ..
if errorlevel 1 exit /b 1
set "LC_PYTHON_SMOKE_PREFIX=%TEMP%\LibreCAD Python smoke %RANDOM%-%RANDOM%"
if exist "%LC_PYTHON_SMOKE_PREFIX%" (
    echo Temporary Python smoke prefix already exists: %LC_PYTHON_SMOKE_PREFIX%
    exit /b 1
)
cmake --install . --config Release --prefix "%LC_PYTHON_SMOKE_PREFIX%"
if errorlevel 1 exit /b 1
set "LC_PYTHON_BUILD_PATH=%PATH%"
pushd "%TEMP%"
if errorlevel 1 exit /b 1
set "PYTHONHOME=%TEMP%\librecad-poison-python-home"
set "PYTHONPATH=%TEMP%\librecad-poison-python-path"
set "LIBRECAD_REQUIRE_CONAN_NOTICES=1"
set "QT_QPA_PLATFORM=offscreen"
set "QT_PLUGIN_PATH="
set "QT_QPA_PLATFORM_PLUGIN_PATH="
set "PATH=%LC_PYTHON_SMOKE_PREFIX%\bin;%SystemRoot%\System32;%SystemRoot%"
"%LC_PYTHON_SMOKE_PREFIX%\bin\librecad.exe" --python-smoke
set "LC_PYTHON_SMOKE_EXIT=%ERRORLEVEL%"
popd
set "PATH=%LC_PYTHON_BUILD_PATH%"
set "LC_PYTHON_BUILD_PATH="
if not "%LC_PYTHON_SMOKE_EXIT%"=="0" exit /b 1
cd /d bin
if errorlevel 1 exit /b 1

dir ..
where makensis.exe
dir ..\CPack*.cmake

cd ..
cpack --verbose -G NSIS --config CPackConfig.cmake -C Release
if errorlevel 1 exit /b 1
