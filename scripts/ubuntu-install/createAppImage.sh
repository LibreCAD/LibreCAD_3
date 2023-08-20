
echo "Begin AppImage building"
cd build
export QTDEPLOY=linuxdeploy-x86_64.AppImage
export QTDEPLOYPLUGIN=linuxdeploy-plugin-qt-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/$QTDEPLOY
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/$QTDEPLOYPLUGIN

chmod a+x linuxdeploy-x86_64.AppImage
chmod a+x linuxdeploy-plugin-qt-x86_64.AppImage

cp -v ../lcUI/ui/icons/librecad.svg AppDir/
cp -v ../desktop/librecad.desktop AppDir/
#sudo cp ../AppImage/librecad.* AppDir/
#sudo cp /usr/local/lib/libdxfrw.so.1 AppDir/usr/lib

sudo LD_LIBRARY_PATH=AppDir/usr/lib/x86_64-linux-gnu/:AppDir/usr/lib64:AppDir/usr/lib ./$QTDEPLOY --appdir AppDir \
--output appimage \
--executable AppDir/usr/bin/librecad \
--desktop-file AppDir/librecad.desktop \
--icon-file AppDir/librecad.svg --plugin qt

