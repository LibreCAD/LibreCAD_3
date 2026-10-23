// R0 CPU-only Qt module audit. Never creates a graphics device or shows a window.
#include <QApplication>
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QPainter>
#include <QPushButton>
#include <QSet>
#include <QSignalSpy>
#include <QSvgRenderer>
#include <QSvgWidget>
#include <QSysInfo>
#include <QTemporaryDir>
#include <QUiLoader>
#include <rhi/qrhi.h>
#include <cstdio>
#include <memory>
#include <stdexcept>
#ifdef Q_OS_MACOS
#include <mach-o/dyld.h>
#include <csignal>
#endif

int main(int argc, char **argv) {
    try {
#ifdef Q_OS_MACOS
        // A closed output pipe must report failure, not terminate on SIGPIPE.
        std::signal(SIGPIPE, SIG_IGN);
#endif
        if (argc != 3 || qgetenv("QT_QPA_PLATFORM") != "offscreen")
            throw std::runtime_error("Require private Qt prefix, plugin directory and explicit offscreen QPA");
        const QString prefix = QFileInfo(QString::fromLocal8Bit(argv[1])).canonicalFilePath();
        const QString plugins = QFileInfo(QString::fromLocal8Bit(argv[2])).canonicalFilePath();
        if (prefix.isEmpty() || !plugins.startsWith(prefix + '/'))
            throw std::runtime_error("Invalid private provider paths");
        QCoreApplication::setLibraryPaths({plugins});
        QApplication app(argc, argv);
        QJsonArray checks;
        QSet<QString> ids;
        auto check = [&](bool passed, const char *id) {
            if (!passed || ids.contains(QString::fromLatin1(id)))
                throw std::runtime_error(id);
            ids.insert(QString::fromLatin1(id));
            checks.append(QString::fromLatin1(id));
        };
        check(QString::fromLatin1(qVersion()) == QString::fromLatin1(QT_VERSION_STR), "qt-build-runtime-version");
        check(QGuiApplication::platformName() == "offscreen", "cpu-offscreen-qpa");
        check(QString::fromLatin1(QRhi::backendName(QRhi::Vulkan)) == "Vulkan", "private-rhi-header-link");
        check(QString::fromLatin1(QOpenGLWidget::staticMetaObject.className()) == "QOpenGLWidget", "legacy-widget-link-only");
        check(QString::fromLatin1(QOpenGLShaderProgram::staticMetaObject.className()) == "QOpenGLShaderProgram", "legacy-opengl-link-only");
        check(QString::fromLatin1(QSvgWidget::staticMetaObject.className()) == "QSvgWidget", "svg-widget-link-only");

        QByteArray ui = R"(<ui version="4.0"><class>Probe</class><widget class="QWidget" name="probe"><widget class="QLabel" name="label"><property name="text"><string>R0 modules</string></property></widget><widget class="QPushButton" name="button"/></widget><resources/><connections/></ui>)";
        QBuffer form(&ui);
        check(form.open(QIODevice::ReadOnly), "form-buffer-open");
        QUiLoader loader;
        std::unique_ptr<QWidget> widget(loader.load(&form));
        check(bool(widget), "uitools-form-load");
        auto *label = widget->findChild<QLabel *>("label");
        auto *button = widget->findChild<QPushButton *>("button");
        check(label && label->text() == "R0 modules" && button, "uitools-properties");
        QSignalSpy spy(button, &QPushButton::clicked);
        check(spy.isValid(), "test-signal-spy-valid");
        button->click();
        check(spy.count() == 1 && spy.at(0).at(0).toBool() == false, "test-signal-observed");

        const QByteArray svg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="4" height="4"><rect width="4" height="4" fill="#ff0000"/></svg>)";
        QSvgRenderer renderer(svg);
        check(renderer.isValid() && renderer.defaultSize() == QSize(4, 4), "svg-document");
        QImage raster(4, 4, QImage::Format_ARGB32_Premultiplied);
        raster.fill(Qt::transparent);
        {
            QPainter painter(&raster);
            renderer.render(&painter);
        }
        check(raster.pixelColor(2, 2) == QColor(255, 0, 0, 255), "svg-cpu-pixel");
        QByteArray svgCopy = svg;
        QBuffer imageBuffer(&svgCopy);
        check(imageBuffer.open(QIODevice::ReadOnly), "svg-buffer-open");
        QImageReader imageReader(&imageBuffer, "svg");
        const auto decoded = imageReader.read();
        check(decoded.size() == QSize(4, 4) && decoded.pixelColor(2, 2) == QColor(255, 0, 0, 255), "svg-image-plugin-pixel");
        QTemporaryDir temporary;
        check(temporary.isValid(), "private-asset-directory");
        const QString iconName = temporary.filePath(QString::fromUtf8("icon ü.svg"));
        QFile iconFile(iconName);
        check(iconFile.open(QIODevice::WriteOnly) && iconFile.write(svg) == svg.size(), "svg-icon-write");
        iconFile.close();
        const auto icon = QIcon(iconName).pixmap(4, 4).toImage();
        check(icon.size() == QSize(4, 4) && icon.pixelColor(2, 2) == QColor(255, 0, 0, 255), "svg-icon-plugin-pixel");

        QSet<QString> loaded;
#ifdef Q_OS_MACOS
        check(_dyld_image_count() > 0, "native-loader-map-readable");
        for (uint32_t index = 0; index < _dyld_image_count(); ++index) {
            const auto name = QString::fromLocal8Bit(_dyld_get_image_name(index));
            if ((name.contains("/Qt") && name.contains(".framework/")) || name.contains("/plugins/"))
                loaded.insert(QFileInfo(name).canonicalFilePath());
        }
#else
        QFile maps("/proc/self/maps");
        check(maps.open(QIODevice::ReadOnly), "native-loader-map-readable");
        for (const QByteArray &line : maps.readAll().split('\n')) {
            const auto slash = line.indexOf('/');
            if (slash < 0) continue;
            const auto name = QString::fromLocal8Bit(line.mid(slash)).replace("\\040", " ");
            if (!name.contains("libQt6") && !name.contains("/qt6/plugins/")) continue;
            loaded.insert(QFileInfo(name).canonicalFilePath());
        }
#endif
        QStringList ordered(loaded.begin(), loaded.end());
        ordered.sort();
        bool privatePaths = !ordered.isEmpty();
        for (const auto &path : ordered) privatePaths &= !path.isEmpty() && path.startsWith(prefix + '/');
        check(privatePaths, "loaded-qt-paths-private");
#ifdef Q_OS_MACOS
        const auto requiredFiles = {"QtCore", "QtGui", "QtWidgets", "QtSvg", "QtSvgWidgets", "QtUiTools",
                                    "QtTest", "QtOpenGL", "QtOpenGLWidgets", "libqoffscreen.dylib",
                                    "libqsvg.dylib", "libqsvgicon.dylib"};
        const char *evidenceKind = "r0-macos-qt-modules";
#else
        const auto requiredFiles = {"libQt6Core.so", "libQt6Gui.so", "libQt6Widgets.so", "libQt6Svg.so",
                                     "libQt6SvgWidgets.so", "libQt6UiTools.so", "libQt6Test.so",
                                     "libQt6OpenGL.so", "libQt6OpenGLWidgets.so", "libqoffscreen.so",
                                     "libqsvg.so", "libqsvgicon.so"};
        const char *evidenceKind = "r0-linux-qt-modules";
#endif
        for (const auto &required : requiredFiles) {
            bool found = false;
            for (const auto &path : ordered) found |= QFileInfo(path).fileName().startsWith(required);
            check(found, required);
        }
        const QJsonObject report{{"schema_version", 1}, {"evidence_kind", evidenceKind},
            {"architecture", QSysInfo::currentCpuArchitecture()},
            {"qt_build_version", QT_VERSION_STR}, {"qt_runtime_version", qVersion()},
            {"qt_prefix", prefix}, {"plugin_directory", plugins}, {"qpa", QGuiApplication::platformName()},
            {"application_created_graphics_device", false}, {"legacy_context_qualified", false},
            {"loaded_qt_paths", QJsonArray::fromStringList(ordered)}, {"checks", checks}};
        const auto json = QJsonDocument(report).toJson(QJsonDocument::Indented);
        if (std::fwrite(json.constData(), 1, size_t(json.size()), stdout) != size_t(json.size()) || std::fflush(stdout))
            throw std::runtime_error("report-write-failed");
        return 0;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "R0 Qt module probe failed: %s\n", error.what());
        return 1;
    }
}
