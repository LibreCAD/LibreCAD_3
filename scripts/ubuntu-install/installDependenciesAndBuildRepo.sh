sudo apt update
sudo apt upgrade -y
# `python3-dev` added for lcadpythonscript (WITH_PYTHONSCRIPT=ON) — phase 1 slice 1.2.
sudo apt install -y -qq cmake qttools5-dev qttools5-dev-tools libqt5opengl5-dev liblua5.3-dev git gcc \
libcairo2-dev libpango-1.0-0 libpango1.0-dev libboost-dev libboost-all-dev libboost-program-options-dev \
libqt5svg5-dev libgtest-dev libeigen3-dev libcurl4-gnutls-dev libgtk-3-dev libglew-dev rapidjson-dev \
libbz2-dev libglfw3-dev libglm-dev libfltk1.3-dev doxygen mkdocs qtchooser freeglut3-dev fuse \
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

git clone --branch LibreCAD_3 https://github.com/LibreCAD/libdxfrw
mkdir -p libdxfrw/release
pushd libdxfrw/release
echo "building dxfrw"
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS=On ..
make -j 4
echo "installing dxfrw"
sudo make install
popd

echo "building LibreCAD"
mkdir build
pushd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS=On
make -j 4

# ---- Run the test suite in CI (phase 1 slice 1.2) ----
# Historically `lcunittest` was BUILT but NEVER RUN in any CI job — every later
# phase's "tests green" gate depends on this being real, not aspirational.
# Uses xvfb-run because some tests spin up the Qt UI.
# `|| true` is DELIBERATELY NOT USED — an unset test failure must fail CI.
if [ -x ./bin/lcunittest ]; then
    echo "running lcunittest"
    xvfb-run -a ./bin/lcunittest
else
    echo "WARNING: bin/lcunittest not built; skipping test run"
fi

echo "installing LibreCAD"
sudo make install DESTDIR=AppDir
sudo cp bin/*.lua bin/*.json AppDir/usr/bin
popd
