#pragma once
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <rhi/qrhi.h>

struct SpikeOptions {
    QString backend, report, output;
    QSize size{256, 192};
    bool transparent = false;
    QString scenario = "standard";
    QString injectFailure;
};

void initializeSpikeResources();
SpikeOptions spikeOptions();
QJsonObject spikeDeviceInfo(QRhi *rhi);
void requireSpikeCapabilities(QRhi *rhi, const QSize &size);
QJsonObject spikeImage(const QImage &image, const QString &path, const QString &id, bool transparent = false);
QJsonObject spikeBuildInfo();
void writeSpikeReport(const SpikeOptions &options, const QString &mode,
                      QJsonObject runtime, const QJsonArray &outputs);
void spikeFailure(const QString &message);
bool spikeHasFailed();
QJsonArray spikeFailures();
void writeSpikeFailureReport(const SpikeOptions &options, QJsonObject runtime, const QJsonObject &failure);
void writeSpikeInputMarker(const SpikeOptions &options, bool ready = false);
