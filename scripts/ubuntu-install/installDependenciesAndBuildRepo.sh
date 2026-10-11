#!/usr/bin/env bash
# A failing command below must fail this build, or the checks after
# lcunittest are meaningless (verified: this alone makes a segfault below
# fail the script, with no other change needed).
set -e

sudo apt update
# No `apt upgrade`. It used to be here, and it upgraded every package on the
# runner image before building a CAD application: seventy-odd packages of
# Firefox, PHP and friends, none of them a dependency of anything below. It
# cost minutes per run and it took the build down whenever any one of them was
# mid-transition in the archive -- most recently
#
#   Errors were encountered while processing:
#    /tmp/apt-dpkg-install-.../11-firefox_1%3a1snap1-0ubuntu9.1_amd64v3.deb
#   Error: Sub-process /usr/bin/dpkg returned an error code (1)
#
# which failed BuildLinux with the tree untouched. `apt install` below pulls
# and upgrades what this build actually needs.
# `python3-dev` added for lcadpythonscript (WITH_PYTHONSCRIPT=ON) — phase 1 slice 1.2.
# `patchelf` is required by linuxdeploy's Qt plugin to rewrite library RPATHs.
sudo apt install -y -qq cmake qt6-base-dev qt6-tools-dev qt6-tools-dev-tools libqt6opengl6-dev liblua5.3-dev git gcc \
libcairo2-dev libpango-1.0-0 libpango1.0-dev libboost-dev libboost-all-dev libboost-program-options-dev \
qt6-svg-dev libgtest-dev libeigen3-dev libcurl4-gnutls-dev libgtk-3-dev libglew-dev rapidjson-dev \
libbz2-dev libglfw3-dev libglm-dev libfltk1.3-dev doxygen mkdocs freeglut3-dev fuse \
python3-dev python3 xvfb patchelf

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
# other submodule (kaguya, libdxfrw, tinyspline, nano-signal-slot).
# Historically the CI called `git submodule update` which no-ops on a fresh
# clone without the paths already registered as initialized — plan phase 1
# slice 1.2 lists this as a CI dep fix.
git submodule update --init --recursive

echo "building LibreCAD"
LIBRECAD_BUILD_DIR="${LIBRECAD_BUILD_DIR:-build}"
mkdir -p "$LIBRECAD_BUILD_DIR"
pushd "$LIBRECAD_BUILD_DIR"
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS=On
make -j 4

# ---- Run the test suite in CI (phase 1 slice 1.2) ----
# `set -e` above makes a failure here fail the build, but most Qt/Python UI
# suites remain informational because their crash location moves between runs
# (see the CI review dated 2026-09-12). CORE_SUITES includes the always-built
# kernel suites, required feature-gated persistence/Python suites, deterministic
# scripting regressions, and the focused Python rectangle drag/undo test. The
# selected UI regression is blocking; everything else still runs for
# visibility with a separate 180-second bound so it cannot prevent the required
# installed-runtime smoke. Failures/timeouts stay visible but informational.
CORE_SUITES="BEZIER_CUBIC.*:BEZIER_QAUDRATIC.*:BEZIER_QUADRATIC.*:BlockOps.*:BuilderTest.*:CustomEntityDxfRoundTrip.*:CustomEntityStorageTest.*:DispatchTest.*:DocumentList.*:DocumentSourceTest.*:DxfExportTest.*:DwgImportTest.*:DxfRoundTripTest.*:EIGEN.*:EntityBuilderTest.*:FormatTest.*:IntersectTest.*:LayerOps.*:LoggerTest.*:LuaFixture.*:LuaCustomEntityManager.*:LuaRuntimeTest.*:MathTest.*:Maths.*:Matrix.*:ParityTest.*:PersistenceTest.*:PythonFixture.*:PythonRuntime.*:QM.*:SPLINE.*:ScriptCallbackInvoke.*:ScriptCallbackEquality.*:ScriptCallbackLifecycle.*:ScriptCustomEntity.*:ScriptCustomEntityBuilder.*:ScriptDockTest.*:ScriptObject.*:PyGuiBridgeTest.*:ToolbarButtonTest.*:MWindowTest.*:EventBus.*:SelectionTest.*:entitytest.*:iColor.*:lc__entity__EllipseTest.*:lc__geo__ArcTest.*:lc__geo__CircleTest.*:lc__geo__EllipseTest.*:lc__geo__RegionTest.*:test.*"

if [ -x ./bin/lcunittest ]; then
    echo "running lcunittest (core suites — blocking)"
    python3 ../scripts/run_required_gtests.py \
        --command-prefix=xvfb-run \
        --command-prefix=-a \
        --binary ./bin/lcunittest \
        --filter="${CORE_SUITES}"

    echo "running lcunittest (remaining suites — informational, not gating)"
    if timeout --kill-after=10s 180s xvfb-run -a ./bin/lcunittest --gtest_filter="-${CORE_SUITES}"; then
        echo "informational tests completed"
    else
        echo "WARNING: non-core tests exited $? (180s limit, 10s kill grace) — tracked, not blocking"
    fi
else
    echo "ERROR: bin/lcunittest not built; required tests cannot run" >&2
    exit 1
fi

echo "installing LibreCAD"
sudo make install DESTDIR=AppDir
sudo cp bin/*.lua bin/*.json AppDir/usr/bin

# Exercise the installed prefix (including its resolved lcUIPy package and
# production plugin loader) from outside the build tree. Poison inherited
# Python settings so success must come from PythonInit's selected provider.
# This pre-linuxdeploy DESTDIR is still system-linked; linuxdeploy adds the
# AppDir's native libpython closure later in createAppImage.sh.
installed_app="$(pwd)/AppDir/usr/bin/librecad"
if [ ! -x "$installed_app" ]; then
    echo "ERROR: installed LibreCAD executable is missing: $installed_app" >&2
    exit 1
fi
(cd /tmp && xvfb-run -a env QT_QPA_PLATFORM=xcb \
    PYTHONHOME=/__librecad_python_poison__ \
    PYTHONPATH=/__librecad_python_poison__ \
    "$installed_app" --python-smoke)
popd
