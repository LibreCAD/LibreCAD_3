
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS=On
make -j 4

sudo make install DESTDIR=AppDir

sudo cp ../AppImage/librecad.* AppDir/
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage

chmod a+x linuxdeploy-x86_64.AppImage
chmod a+x linuxdeploy-plugin-qt-x86_64.AppImage

sudo cp ../AppImage/librecad.* AppDir/

sudo cp ../../libdxfrw/release/libdxfrw.so.1 AppDir/usr/lib/ #can't find this library in the Runner

sudo LD_LIBRARY_PATH=AppDir/usr/lib/x86_64-linux-gnu/:AppDir/usr/lib64 ./linuxdeploy-x86_64.AppImage --appdir AppDir --output appimage --executable AppDir/usr/bin/librecad --desktop-file AppDir/librecad.desktop --icon-file AppDir/librecad.svg --plugin qt