sudo apt-get install qttools5-dev qttools5-dev-tools libqt5opengl5-dev liblua5.2-dev git g++ libcairo2-dev libpango-1.0-0 libpango1.0-dev libboost-dev libboost-log-dev libboost-program-options-dev libqt5svg5-dev libgtest-dev libeigen3-dev libcurl4-gnutls-dev libgtk-3-dev libglew-dev rapidjson-dev libbz2-dev libglfw3-dev libglm-dev cmake

sudo apt remove --purge --auto-remove cmake
sudo apt update && \
sudo apt install -y software-properties-common lsb-release && \
sudo apt clean all
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
sudo apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"
sudo apt update
sudo apt install kitware-archive-keyring
sudo rm /etc/apt/trusted.gpg.d/kitware.gpg
sudo apt update
sudo apt install cmake

cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make

cd lib #potser no fa falta.
sudo cp *.a /usr/lib

cd
git clone --branch LibreCAD_3 https://github.com/LibreCAD/libdxfrw
cd libdxfrw
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON ..
make
sudo make install

export libdxfrwPath=`pwd` #we need to export the path to the libdxfrw folder for linuxdeploy to get libdxfrw sistem libraries

