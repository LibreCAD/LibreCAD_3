#!/usr/bin/env bash
set -euo pipefail

# The Ubuntu build enables Python scripting and CMake's install rules stage
# the selected CPython provider's standard library and dynamic modules in
# AppDir before this script runs linuxdeploy. linuxdeploy then collects native
# runtime dependencies, including libpython; this helper does not select or
# download a Python provider.
#
# The Ubuntu workflow separately smokes the post-linuxdeploy AppDir and final
# AppImage with inherited Python paths poisoned. Keep the platform gate open
# until the hosted job and uploaded runtime inventory pass; local packaging
# does not establish native x86_64 launcher or host-independent closure.
echo "Begin AppImage building"
cd build || exit 1
export QTDEPLOY=linuxdeploy-x86_64.AppImage
export QTDEPLOYPLUGIN=linuxdeploy-plugin-qt-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/$QTDEPLOY
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/$QTDEPLOYPLUGIN

chmod a+x linuxdeploy-x86_64.AppImage
chmod a+x linuxdeploy-plugin-qt-x86_64.AppImage

cp -v ../lcUI/ui/icons/librecad.svg AppDir/
cp -v ../desktop/librecad.desktop AppDir/

#sudo cp ../AppImage/librecad.* AppDir/

# Dependencies of the staged Python extension modules that only some Ubuntu
# releases have.  Ubuntu 24.04's _tkinter also links BLT (tk8.6-blt2.5); the
# 26.04 build's did not.  Without it the clean-runtime ELF closure audit fails
# on _tkinter, and linuxdeploy refuses a --library that does not exist.
release_libraries=()
if [[ -e /usr/lib/libBLT.2.5.so.8.6 ]]; then
    release_libraries+=(--library=/usr/lib/libBLT.2.5.so.8.6)
fi

sudo \
LD_LIBRARY_PATH="AppDir/usr/lib/x86_64-linux-gnu/:AppDir/usr/lib64:AppDir/usr/lib" \
VERSION="$(git describe)" \
"./$QTDEPLOY" --appdir AppDir \
--library=/usr/lib/x86_64-linux-gnu/libssl.so.3 \
--library=/usr/lib/x86_64-linux-gnu/libcrypto.so.3 \
--library=/usr/lib/x86_64-linux-gnu/libsqlite3.so.0 \
--library=/usr/lib/x86_64-linux-gnu/libffi.so.8 \
--library=/usr/lib/x86_64-linux-gnu/libz.so.1 \
--library=/usr/lib/x86_64-linux-gnu/libharfbuzz.so.0 \
--library=/usr/lib/x86_64-linux-gnu/libgraphite2.so.3 \
--library=/usr/lib/x86_64-linux-gnu/libreadline.so.8 \
--library=/usr/lib/x86_64-linux-gnu/libgdbm.so.6 \
--library=/usr/lib/x86_64-linux-gnu/libtcl8.6.so \
--library=/usr/lib/x86_64-linux-gnu/libtk8.6.so \
--library=/usr/lib/x86_64-linux-gnu/libXft.so.2 \
--library=/usr/lib/x86_64-linux-gnu/libXss.so.1 \
"${release_libraries[@]}" \
--output appimage \
--executable AppDir/usr/bin/librecad \
--desktop-file AppDir/librecad.desktop \
--icon-file AppDir/librecad.svg --plugin qt \
--custom-apprun ../scripts/ubuntu-install/AppRun

# The pre-linuxdeploy install smoke intentionally accepts the matching system
# libpython. After linuxdeploy, exercise the AppDir launcher from outside the
# checkout and require the loaded runtime to come from this packaged tree.
appdir_root="$(realpath AppDir)"
appdir_launcher="$appdir_root/AppRun"
if [[ ! -x "$appdir_launcher" ]]; then
    echo "ERROR: linuxdeploy did not create an executable AppRun: $appdir_launcher" >&2
    exit 1
fi
smoke_settings="$(mktemp -d /tmp/librecad-appdir-python-smoke.XXXXXX)"
trap 'rm -rf "$smoke_settings"' EXIT
(
    cd /tmp
    xvfb-run -a env \
        -u QT_PLUGIN_PATH \
        -u QT_QPA_PLATFORM_PLUGIN_PATH \
        QT_QPA_PLATFORM=xcb \
        PYTHONHOME=/__librecad_python_poison__ \
        PYTHONPATH=/__librecad_python_poison__ \
        LIBRECAD_REQUIRE_BUNDLED_PYTHON=1 \
        LIBRECAD_SETTINGS_DIR="$smoke_settings" \
        "$appdir_launcher" --python-smoke
)

mv LibreCAD*.AppImage "LibreCAD3-$(git describe --always).AppImage"
