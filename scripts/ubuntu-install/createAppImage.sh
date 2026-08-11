
# Phase 6 PR-6.3 — Ubuntu AppImage packaging notes.
#
# The AppImage is built via linuxdeploy + the Qt plugin.  When the
# librecad binary is linked against libpython3.X.so (WITH_PYTHONSCRIPT=ON
# on the build machine — see installDependenciesAndBuildRepo.sh which
# installs python3-dev), linuxdeploy automatically bundles that shared
# library into the AppImage.
#
# HOWEVER: the CPython STANDARD LIBRARY (~30 MB of .py files under
# /usr/lib/python3.X/) is not a shared-library dependency and is NOT
# bundled.  A user running the AppImage on a system without a matching
# Python 3.X installation will get import errors on `import os`
# (etc.) when Python scripts try to use stdlib modules.
#
# TWO acceptable resolutions per the phase-6 sub-plan (choose one for
# each release):
#   (a) Ship the AppImage with WITH_PYTHONSCRIPT=OFF for now and
#       document the Lua-only scripting limitation for AppImage
#       distribution.  Users wanting Python can build from source.
#   (b) Add a full linuxdeploy-plugin-python step that bundles libpython
#       + a subset of stdlib (~30-50 MB extra AppImage size).
#
# This script currently takes path (a) implicitly — no linuxdeploy
# python plugin is invoked.  The librecad binary in the AppImage
# still contains lcadpythonscript, so scripts that avoid stdlib
# imports will work; the practical limitation is stdlib.  Future
# work (deferred per sub-plan): add the linuxdeploy python plugin.
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

sudo LD_LIBRARY_PATH=AppDir/usr/lib/x86_64-linux-gnu/:AppDir/usr/lib64:AppDir/usr/lib \
VERSION=`git describe ` ./$QTDEPLOY --appdir AppDir \
--output appimage \
--executable AppDir/usr/bin/librecad \
--desktop-file AppDir/librecad.desktop \
--icon-file AppDir/librecad.svg --plugin qt
mv LibreCAD*.AppImage LibreCAD3-$(git describe --always).AppImage
