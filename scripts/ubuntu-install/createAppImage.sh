# Phase 6 PR-6.3 — Ubuntu AppImage packaging notes.
#
# The AppImage is built via linuxdeploy + the Qt plugin.  The librecad binary
# is linked against libpython3.X.so — installDependenciesAndBuildRepo.sh
# installs python3-dev and configures with the default WITH_PYTHONSCRIPT=ON —
# so linuxdeploy bundles that shared library automatically.
#
# The CPython STANDARD LIBRARY is not a shared-library dependency, so
# linuxdeploy does not bundle it, and an embedded interpreter without one does
# not fail softly.  lcUI/mainwindow.cpp constructs the interpreter during
# MainWindow construction, unconditionally, and CPython that cannot find its
# stdlib calls Py_FatalError:
#
#   Fatal Python error: init_fs_encoding: failed to get the Python codec of
#   the filesystem encoding
#   ModuleNotFoundError: No module named 'encodings'
#
# which aborts the process before any C++ around it can react.  The AppImage
# therefore started only on machines that happened to have a matching CPython
# installed under /usr — which is the one thing an AppImage is supposed not to
# need.
#
# So the stdlib is copied in below and PYTHONHOME is pointed at it.  This is
# path (b) of the phase-6 sub-plan; the header used to claim path (a) (ship
# with WITH_PYTHONSCRIPT=OFF) while the build had Python on the whole time.
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

# The interpreter's own library, and the environment that lets it find it.
# linuxdeploy's AppRun sources every script in apprun-hooks/ before exec, which
# is the same mechanism its Qt plugin uses to set QT_PLUGIN_PATH.
PYTHON_TAG=$(python3 -c 'import sys; print("python%d.%d" % sys.version_info[:2])')
PYTHON_STDLIB=$(python3 -c 'import sysconfig; print(sysconfig.get_path("stdlib"))')
if [ -d "$PYTHON_STDLIB" ]; then
    echo "Bundling $PYTHON_STDLIB as usr/lib/$PYTHON_TAG"
    mkdir -p "AppDir/usr/lib/$PYTHON_TAG"
    cp -a "$PYTHON_STDLIB/." "AppDir/usr/lib/$PYTHON_TAG/"
    # The test package and the bytecode caches are most of the size and none
    # of the use; idlelib and tkinter need a Tk that is not bundled either.
    rm -rf "AppDir/usr/lib/$PYTHON_TAG/test" \
           "AppDir/usr/lib/$PYTHON_TAG/idlelib" \
           "AppDir/usr/lib/$PYTHON_TAG/tkinter"
    find "AppDir/usr/lib/$PYTHON_TAG" -name __pycache__ -type d -prune -exec rm -rf {} +

    mkdir -p AppDir/apprun-hooks
    cat > AppDir/apprun-hooks/python-hook.sh <<'HOOK'
# Point the embedded interpreter at the stdlib bundled beside it, so it does
# not go looking in the host's /usr and abort when there is nothing there.
#
# APPDIR is exported by the AppImage runtime; this_dir is what linuxdeploy's
# own AppRun wrapper sets, and is what is there when someone extracts the
# AppDir and runs it directly.
export PYTHONHOME="${APPDIR:-$this_dir}/usr"
export PYTHONDONTWRITEBYTECODE=1
HOOK
else
    echo "WARNING: no CPython stdlib found at $PYTHON_STDLIB;" \
         "the AppImage will abort at startup on a host without one"
fi
#sudo cp ../AppImage/librecad.* AppDir/

sudo LD_LIBRARY_PATH=AppDir/usr/lib/x86_64-linux-gnu/:AppDir/usr/lib64:AppDir/usr/lib \
VERSION=`git describe ` ./$QTDEPLOY --appdir AppDir \
--output appimage \
--executable AppDir/usr/bin/librecad \
--desktop-file AppDir/librecad.desktop \
--icon-file AppDir/librecad.svg --plugin qt
mv LibreCAD*.AppImage LibreCAD3-$(git describe --always).AppImage
