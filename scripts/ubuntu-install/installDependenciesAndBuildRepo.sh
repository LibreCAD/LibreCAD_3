# A failing command below must fail this build, or the checks after
# lcunittest are meaningless (verified: this alone makes a segfault below
# fail the script, with no other change needed).
set -e

sudo apt update
sudo apt upgrade -y
# `python3-dev` added for lcadpythonscript (WITH_PYTHONSCRIPT=ON) — phase 1 slice 1.2.
sudo apt install -y -qq cmake qt6-base-dev qt6-tools-dev qt6-tools-dev-tools libqt6opengl6-dev liblua5.3-dev git gcc \
libcairo2-dev libpango-1.0-0 libpango1.0-dev libboost-dev libboost-all-dev libboost-program-options-dev \
qt6-svg-dev libgtest-dev libeigen3-dev libcurl4-gnutls-dev libgtk-3-dev libglew-dev rapidjson-dev \
libbz2-dev libglfw3-dev libglm-dev libfltk1.3-dev doxygen mkdocs freeglut3-dev fuse \
python3-dev python3

#This is for versions older that 20.04, like ubuntu 18.04

#sudo apt remove --purge --auto-remove cmake
#sudo apt update && \
#sudo apt install -y software-properties-common lsb-release && \
#sudo apt clean all
#wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
#sudo apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"
#sudo apt update
#sudo apt install kitware-archive-keyring
#sudo rm /etc/apt/trusted.gpg.d/kitware.gpg
#sudo apt update
#sudo apt install cmake
echo on
echo "building LibreCAD"
# `--init --recursive` fixed for lcadpythonscript/third_party/pybind11 and every
# other submodule (kaguya, libopencad, tinyspline, nano-signal-slot).
# Historically the CI called `git submodule update` which no-ops on a fresh
# clone without the paths already registered as initialized — plan phase 1
# slice 1.2 lists this as a CI dep fix.
git submodule update --init --recursive

LIBDXFRW_REVISION=f7debf4c5f30734a70c5e828cf05247aabc2d04b
if [ -d libdxfrw/.git ]; then
    libdxfrw_actual_revision=$(git -C libdxfrw rev-parse HEAD)
    if [ "$libdxfrw_actual_revision" != "$LIBDXFRW_REVISION" ]; then
        echo "libdxfrw revision mismatch: expected $LIBDXFRW_REVISION, got $libdxfrw_actual_revision" >&2
        exit 1
    fi
else
    git clone --branch LibreCAD_3 https://github.com/LibreCAD/libdxfrw
    git -C libdxfrw checkout --detach "$LIBDXFRW_REVISION"
fi
bash scripts/patch-libdxfrw.sh "$PWD/libdxfrw"
mkdir -p libdxfrw/release
pushd libdxfrw/release
echo "building dxfrw"
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_CXX_FLAGS=-Wno-error=misleading-indentation \
    -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS=On ..
make -j 4
echo "installing dxfrw"
sudo make install
popd

echo "building LibreCAD"
build_dir="${LIBRECAD_BUILD_DIR:-build}"
mkdir -p "$build_dir"
pushd "$build_dir"
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS=On \
    -DWITH_BUNDLED_PYTHON_RUNTIME=ON
make -j 4

# ---- Run required and informational tests in CI ----
# CORE_SUITES contains the always-built kernel tests plus the required Python,
# persistence, parity, custom-entity, and deterministic UI regression suites.
# Every release-relevant Python/scripting regression is blocking. The shared
# runner verifies selected and executed counts and rejects skipped tests.
# Non-core tests still run for visibility, but remain informational.
CORE_SUITES="BEZIER_CUBIC.*:BEZIER_QAUDRATIC.*:BEZIER_QUADRATIC.*:BlockOps.*:BuilderTest.*:CustomEntityDxfRoundTrip.*:CustomEntityStorageTest.*:DispatchTest.*:DocumentList.*:DxfExportTest.*:EIGEN.*:EntityBuilderTest.*:EventBus.*:IntersectTest.*:LayerOps.*:LuaCustomEntityManager.*:LuaRuntimeTest.*:MathTest.*:Maths.*:Matrix.*:MWindowTest.*:ParityTest.*:PersistenceTest.*:PythonFixture.*:PythonRuntime.*:PyGuiBridgeTest.*:QM.*:SPLINE.*:ScriptCallbackEquality.*:ScriptCallbackInvoke.*:ScriptCallbackLifecycle.*:ScriptCustomEntity.*:ScriptCustomEntityBuilder.*:ScriptCustomEntityLuaRegistration.*:ScriptDockTest.Python*:ScriptValueRoundTrip.*:SelectionTest.*:entitytest.*:iColor.*:lc__entity__EllipseTest.*:lc__geo__ArcTest.*:lc__geo__CircleTest.*:lc__geo__EllipseTest.*:lc__geo__RegionTest.*:test.*"

if [ ! -x ./bin/lcunittest ]; then
    echo "ERROR: required lcunittest executable was not built" >&2
    exit 1
fi

echo "running lcunittest (blocking kernel/Python/scripting/UI suites)"
python3 ../scripts/run_required_gtests.py \
    --command-prefix=xvfb-run --command-prefix=-a \
    --binary ./bin/lcunittest --filter="${CORE_SUITES}"

echo "running lcunittest (remaining suites — informational, not gating)"
xvfb-run -a ./bin/lcunittest --gtest_filter="-${CORE_SUITES}" || \
    echo "WARNING: a non-core test failed or crashed above — tracked, not blocking"

echo "installing LibreCAD"
sudo make install DESTDIR=AppDir
sudo cp bin/*.lua bin/*.json AppDir/usr/bin

# Exercise the installed prefix (including its resolved lcUIPy package and
# production plugin loader) from outside the build tree. Poison inherited
# Python settings so success must come from PythonInit's selected provider.
installed_app="$(pwd)/AppDir/usr/bin/librecad"
if [ ! -x "$installed_app" ]; then
    echo "ERROR: installed LibreCAD executable is missing: $installed_app" >&2
    exit 1
fi
(cd /tmp && env QT_QPA_PLATFORM=offscreen \
    PYTHONHOME=/__librecad_python_poison__ \
    PYTHONPATH=/__librecad_python_poison__ \
    LIBRECAD_REQUIRE_BUNDLED_PYTHON=1 \
    "$installed_app" --python-smoke)
popd
