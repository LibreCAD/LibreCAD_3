
win32 {
    include(settings_windows.pri)
}
unix {
    macx {
        include(settings_macx.pri)
    } else {
        include(settings_linux.pri)
    }
}

include(common.pri)
