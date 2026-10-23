#include "support.h"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QSaveFile>
#include <QSysInfo>
#include <stdexcept>

void initializeSpikeResources() {
    Q_INIT_RESOURCE(spike_shaders);
    Q_INIT_RESOURCE(spike_build);
    Q_INIT_RESOURCE(spike_fixture);
}

namespace {
// R0 hosts and QRhi callbacks run on the application's owner thread.
QJsonArray failures;
void requireSuccess() {
    if (spikeHasFailed()) throw std::runtime_error("Terminal spike failure forbids successful output");
}
QByteArray read(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) throw std::runtime_error(("Cannot read " + path).toStdString());
    return f.readAll();
}
QString sha256(const QByteArray &data) {
    return QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}
void save(const QString &path, const QByteArray &data) {
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size() || !file.commit())
        throw std::runtime_error(("Cannot write " + path).toStdString());
}
}

SpikeOptions spikeOptions() {
    QCommandLineParser p;
    p.setApplicationDescription("LibreCAD R0 RHI spike; no fallback, no production acceptance");
    p.addHelpOption();
    p.addOption({"backend", "Required graphics API: metal (macOS) or vulkan (Windows/Linux)", "api"});
    p.addOption({"renderer-report", "New app-only spike report", "path"});
    p.addOption({"render-output", "Output PNG path", "path"});
    p.addOption({"width", "Offscreen pixel width (16..4096)", "pixels", "256"});
    p.addOption({"height", "Offscreen pixel height (16..4096)", "pixels", "192"});
    p.addOption({"background", "Target background: opaque or transparent", "kind", "opaque"});
    p.addOption({"scenario", "Proof scenario: standard, lifecycle, offscreen recovery or widget reparent", "kind", "standard"});
    p.addOption({"inject-failure", "Standard widget terminal failure: render, readback or shutdown", "point"});
    p.process(*QCoreApplication::instance());
    SpikeOptions o{p.value("backend"), p.value("renderer-report"), p.value("render-output")};
    if (p.value("background") != "opaque" && p.value("background") != "transparent")
        throw std::runtime_error("Background must be opaque or transparent");
    o.transparent = p.value("background") == "transparent";
    o.scenario = p.value("scenario");
    o.injectFailure = p.value("inject-failure");
    if (!o.injectFailure.isEmpty() && (o.scenario != "standard" ||
        (o.injectFailure != "render" && o.injectFailure != "readback" && o.injectFailure != "shutdown")))
        throw std::runtime_error("Failure injection requires standard widget render, readback or shutdown");
    if (o.scenario != "standard" && o.scenario != "lifecycle" && o.scenario != "recovery" && o.scenario != "reparent")
        throw std::runtime_error("Scenario must be standard, lifecycle, recovery or reparent");
    bool widthOK, heightOK;
    o.size = {p.value("width").toInt(&widthOK), p.value("height").toInt(&heightOK)};
    if (!widthOK || !heightOK || o.size.width() < 16 || o.size.height() < 16 ||
        o.size.width() > 4096 || o.size.height() > 4096 || o.report.isEmpty() || o.output.isEmpty())
        throw std::runtime_error("Report/output paths and dimensions in 16..4096 are required");
#ifdef Q_OS_MACOS
    if (o.backend != "metal") throw std::runtime_error("This native macOS spike requires --backend=metal");
#else
    if (o.backend != "vulkan") throw std::runtime_error("This platform spike requires --backend=vulkan");
#endif
    if (!QDir().mkpath(QFileInfo(o.output).absolutePath()) || !QDir().mkpath(QFileInfo(o.report).absolutePath()))
        throw std::runtime_error("Cannot create output directory");
    return o;
}

QJsonObject spikeBuildInfo() {
    QJsonObject result = QJsonDocument::fromJson(read(":/spike/build.json")).object();
    QJsonObject shaders;
    for (const QString &name : {QString("scene.vert.qsb"), QString("scene.frag.qsb")})
        shaders.insert(name, sha256(read(":/spike/" + name)));
    result.insert("shaders", shaders);
    result.insert("fixture_sha256", sha256(read(":/spike/fixture.json")));
    result.insert("compile_qt_version", QT_VERSION_STR);
    result.insert("configuration", QString::fromLatin1(LC_RHI_SPIKE_CONFIG));
    return result;
}

QJsonObject spikeDeviceInfo(QRhi *rhi) {
    if (!rhi) throw std::runtime_error("No initialized RHI device");
    QString api;
    if (rhi->backend() == QRhi::Metal) api = "metal";
    else if (rhi->backend() == QRhi::Vulkan) api = "vulkan";
    else throw std::runtime_error("Unexpected RHI backend (fallback rejected)");
    const auto info = rhi->driverInfo();
    QString kind = "unknown";
    switch (info.deviceType) {
    case QRhiDriverInfo::CpuDevice: kind = "software"; break;
    case QRhiDriverInfo::IntegratedDevice:
    case QRhiDriverInfo::DiscreteDevice:
    case QRhiDriverInfo::ExternalDevice: kind = "hardware"; break;
    case QRhiDriverInfo::VirtualDevice: kind = "virtual"; break;
    case QRhiDriverInfo::UnknownDevice: break;
    }
    QJsonArray counts;
    for (int count : rhi->supportedSampleCounts()) counts.append(count);
    return {{"actual_api", api}, {"device_name", QString::fromUtf8(info.deviceName)},
            {"device_class", kind}, {"vendor_id", QString::number(info.vendorId)},
            {"device_id", QString::number(info.deviceId)},
            {"driver_version", QJsonValue::Null},
            {"metadata_note", "QRhi has no universal driver-version query; zero IDs may be unavailable"},
            {"qt_version", qVersion()}, {"os", QSysInfo::productType()},
            {"architecture", QSysInfo::currentCpuArchitecture()},
            {"qpa", QGuiApplication::platformName()}, {"format", "RGBA8"},
            {"sample_count", 1}, {"alpha", "opaque-premultiplied-blending"},
            {"color_space", "SDR-unorm-no-sRGB-transfer"},
            {"capabilities", QJsonObject{{"min_texture_size", rhi->resourceLimit(QRhi::TextureSizeMin)},
                                         {"max_texture_size", rhi->resourceLimit(QRhi::TextureSizeMax)},
                                         {"max_uniform_buffer_range", rhi->resourceLimit(QRhi::MaxUniformBufferRange)},
                                         {"sample_counts", counts},
                                         {"rgba8_render_target", rhi->isTextureFormatSupported(QRhiTexture::RGBA8,
                                             QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource)},
                                         {"rgba8", rhi->isTextureFormatSupported(QRhiTexture::RGBA8)}}}};
}

void requireSpikeCapabilities(QRhi *rhi, const QSize &size) {
    if (!rhi || !rhi->isTextureFormatSupported(QRhiTexture::RGBA8) ||
        !rhi->isTextureFormatSupported(QRhiTexture::RGBA8,
            QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource) ||
        !rhi->supportedSampleCounts().contains(1) || rhi->resourceLimit(QRhi::MaxUniformBufferRange) < 64)
        throw std::runtime_error("Device does not meet spike RGBA8/readback/single-sample/uniform requirements");
    const int minimum = rhi->resourceLimit(QRhi::TextureSizeMin);
    const int maximum = rhi->resourceLimit(QRhi::TextureSizeMax);
    if (minimum < 1 || minimum > 16 || maximum < 256 || size.width() < minimum || size.height() < minimum ||
        size.width() > maximum || size.height() > maximum)
        throw std::runtime_error("Target or atlas dimensions exceed spike device texture limits");
}

QJsonObject spikeImage(const QImage &input, const QString &path, const QString &id, bool transparent) {
    requireSuccess();
    if (input.isNull()) throw std::runtime_error("Null readback image");
    const QImage image = input.convertToFormat(QImage::Format_RGBA8888);
    if (!image.save(path, "PNG")) throw std::runtime_error("PNG write failed");
    QByteArray raw;
    raw.reserve(image.width() * image.height() * 4);
    for (int y = 0; y < image.height(); ++y)
        raw.append(reinterpret_cast<const char *>(image.constScanLine(y)), image.width() * 4);
    const QString rawPath = path + ".rgba";
    save(rawPath, raw);
    return {{"id", id}, {"image", QFileInfo(path).fileName()}, {"image_sha256", sha256(read(path))},
            {"rgba", QFileInfo(rawPath).fileName()}, {"rgba_sha256", sha256(raw)},
            {"width", image.width()}, {"height", image.height()}, {"stride", image.width() * 4},
            {"channels", "RGBA"}, {"origin", "top-left"}, {"alpha", transparent ? "straight" : "opaque"}};
}

void writeSpikeReport(const SpikeOptions &options, const QString &mode,
                      QJsonObject runtime, const QJsonArray &outputs) {
    requireSuccess();
    runtime.insert("requested_api", options.backend);
    runtime.insert("mode", mode);
    runtime.insert("background", options.transparent ? "transparent" : "opaque");
    runtime.insert("scenario", options.scenario);
    runtime.insert("alpha", options.transparent ? "premultiplied-blending-straight-readback" : "opaque-premultiplied-blending");
    const QJsonObject report{{"schema_version", 3}, {"evidence_kind", "r0-spike"},
                             {"fixture", "cad-rhi-spike-v2"}, {"build", spikeBuildInfo()},
                             {"runtime", runtime}, {"outputs", outputs}};
    save(options.report, QJsonDocument(report).toJson());
}

void spikeFailure(const QString &message) {
    failures.append(message);
    qCritical("RHI spike failed: %s", qPrintable(message));
    if (QCoreApplication::instance()) QCoreApplication::exit(1);
}

bool spikeHasFailed() { return !failures.isEmpty(); }
QJsonArray spikeFailures() { return failures; }

void writeSpikeInputMarker(const SpikeOptions &options) {
    const QJsonObject marker{{"key", "F12"}, {"qpa", QGuiApplication::platformName()},
                            {"source", "spontaneous-key-event"}, {"owner_generation", 1}};
    save(QFileInfo(options.report).dir().filePath("wayland-input.json"), QJsonDocument(marker).toJson());
}

void writeSpikeFailureReport(const SpikeOptions &options, QJsonObject runtime, const QJsonObject &failure) {
    if (!spikeHasFailed()) throw std::runtime_error("Failure report requires a terminal failure");
    runtime.insert("requested_api", options.backend);
    runtime.insert("mode", "widget");
    runtime.insert("scenario", options.scenario);
    runtime.insert("background", options.transparent ? "transparent" : "opaque");
    runtime.insert("alpha", options.transparent ? "premultiplied-blending-straight-readback" : "opaque-premultiplied-blending");
    const QJsonObject report{{"schema_version", 3}, {"evidence_kind", "r0-spike-failure"},
        {"fixture", "cad-rhi-spike-v2"}, {"build", spikeBuildInfo()}, {"runtime", runtime},
        {"failure", failure}, {"errors", spikeFailures()}, {"outputs", QJsonArray{}}};
    save(QFileInfo(options.report).dir().filePath("failure.json"), QJsonDocument(report).toJson());
}
