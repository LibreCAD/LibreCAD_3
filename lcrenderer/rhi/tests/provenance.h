#pragma once
#include <QFileInfo>
#include <QSet>
#include <QSysInfo>
#ifdef Q_OS_MACOS
#include <mach-o/dyld.h>
#elif defined(Q_OS_WIN)
#include <QtCore/qt_windows.h>
#endif

// Actual loaded Qt images, not QLibraryInfo's configured installation path.
// This test-only receipt does not claim complete native dependency closure.
static QJsonObject rhiQtModules() {
    QSet<QString> paths;
#ifdef Q_OS_MACOS
    for (uint32_t i = 0; i < _dyld_image_count(); ++i) {
        const auto path = QString::fromLocal8Bit(_dyld_get_image_name(i));
        if (path.contains("/Qt") && path.contains(".framework/")) paths.insert(path);
    }
#elif defined(Q_OS_WIN)
    for (const auto* address : {reinterpret_cast<const void*>(&qVersion),
                               reinterpret_cast<const void*>(&QGuiApplication::platformName)}) {
        HMODULE module = nullptr;
        check(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
              reinterpret_cast<LPCWSTR>(address), &module), "Missing actual Qt symbol module");
        wchar_t path[32768];
        const auto size = GetModuleFileNameW(module, path, 32768);
        check(size > 0 && size < 32768, "Unavailable/truncated loaded Qt module path");
        paths.insert(QString::fromWCharArray(path, int(size)));
    }
#else
    QFile maps("/proc/self/maps");
    check(maps.open(QIODevice::ReadOnly), "Unavailable native loader map");
    for (const auto& line : maps.readAll().split('\n')) {
        const auto slash = line.indexOf('/');
        if (slash < 0) continue;
        const auto path = QString::fromLocal8Bit(line.mid(slash)).replace("\\040", " ");
        if (path.contains("libQt6")) paths.insert(path);
    }
#endif
    QJsonObject result;
    for (const auto& path : paths) {
        const auto canonical = QFileInfo(path).canonicalFilePath();
        QFile file(canonical);
        check(!canonical.isEmpty() && file.open(QIODevice::ReadOnly), "Unavailable loaded Qt image");
        QCryptographicHash hash(QCryptographicHash::Sha256);
        check(hash.addData(&file), "Cannot hash loaded Qt image");
        result.insert(canonical, QString::fromLatin1(hash.result().toHex()));
    }
    check(result.size() >= 2, "Incomplete actual Qt image receipt");
    return result;
}
