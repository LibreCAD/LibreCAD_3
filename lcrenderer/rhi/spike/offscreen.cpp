#include "scene.h"
#include "support.h"
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QStringList>
#include <QVersionNumber>
#include <rhi/qrhi_platform.h>
#include <stdexcept>
#include <utility>

namespace {
class InjectedFault : public std::runtime_error {
public:
    explicit InjectedFault(const QString &point) : std::runtime_error(("Injected host failure: " + point).toStdString()), point(point) {}
    QString point;
};

struct Attempt {
    int generation, creations = 0, destructions = 0, actualSubmit = -1, effectiveSubmit = -1;
    QString fault, api, deviceName, deviceClass;
    bool started = false, completed = false, accepted = false;
    QJsonObject json() const {
        return {{"owner_generation", generation}, {"owned_rhi_creations", creations}, {"owned_rhi_destructions", destructions},
                {"actual_api", api}, {"device_name", deviceName}, {"device_class", deviceClass},
                {"frame_started", started}, {"actual_submit_result", actualSubmit},
                {"effective_submit_result", effectiveSubmit}, {"readback_completed", completed}, {"image_accepted", accepted}};
    }
    [[noreturn]] void fail(const QString &point, const char *message) const {
        if (fault == point) throw InjectedFault(point);
        throw std::runtime_error(message);
    }
};

struct DeviceDeleter {
    Attempt *attempt;
    void operator()(QRhi *device) const { delete device; ++attempt->destructions; }
};

class OffscreenFrame {
public:
    OffscreenFrame(QRhi *device, Attempt &attempt) : device(device) {
        const auto status = attempt.fault == "begin-frame" ? QRhi::FrameOpError : device->beginOffscreenFrame(&cb);
        if (status != QRhi::FrameOpSuccess) attempt.fail("begin-frame", "Cannot begin offscreen frame");
        active = true;
        attempt.started = true;
    }
    ~OffscreenFrame() { if (active) device->endOffscreenFrame(); }
    QRhi::FrameOpResult end() { active = false; return device->endOffscreenFrame(); }
    QRhiCommandBuffer *cb = nullptr;
private:
    QRhi *device;
    bool active = false;
};
}

std::pair<QJsonObject, QImage> renderOwned(const SpikeOptions &options, Attempt &attempt) {
        QElapsedTimer timer;
        timer.start();
        // These CPU owners outlive even a failing device's teardown callbacks.
        bool completed = false;
        QRhiReadbackResult readback;
#if !defined(Q_OS_MACOS) && QT_CONFIG(vulkan)
        QVulkanInstance instance; // Outlives the device using it.
#endif
        std::unique_ptr<QRhi, DeviceDeleter> rhi(nullptr, DeviceDeleter{&attempt});
#ifdef Q_OS_MACOS
        QRhiMetalInitParams params;
        if (attempt.fault != "device-create") rhi.reset(QRhi::create(QRhi::Metal, &params));
#elif QT_CONFIG(vulkan)
        // QRhi expects 1.1 when available (including promoted extension dependencies).
        // Explicit 1.0 remains diagnostic-only on older loaders; never fake support.
        instance.setApiVersion(instance.supportedApiVersion() >= QVersionNumber(1, 1)
                               ? QVersionNumber(1, 1) : QVersionNumber(1, 0));
        instance.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
        if (!instance.create()) throw std::runtime_error("Vulkan instance creation failed");
        QRhiVulkanInitParams params;
        params.inst = &instance;
        if (attempt.fault != "device-create") rhi.reset(QRhi::create(QRhi::Vulkan, &params));
#endif
        if (!rhi) attempt.fail("device-create", "Requested RHI device creation failed (no fallback)");
        ++attempt.creations;
        auto runtime = spikeDeviceInfo(rhi.get());
        attempt.api = runtime["actual_api"].toString();
        attempt.deviceName = runtime["device_name"].toString();
        attempt.deviceClass = runtime["device_class"].toString();
        requireSpikeCapabilities(rhi.get(), options.size);
        auto texture = std::unique_ptr<QRhiTexture>(rhi->newTexture(QRhiTexture::RGBA8, options.size, 1,
                                          QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
        if (attempt.fault == "texture-create" || !texture->create())
            attempt.fail("texture-create", "Offscreen texture creation failed");
        std::unique_ptr<QRhiRenderPassDescriptor> pass;
        auto target = std::unique_ptr<QRhiTextureRenderTarget>(rhi->newTextureRenderTarget({texture.get()}));
        pass.reset(target->newCompatibleRenderPassDescriptor());
        target->setRenderPassDescriptor(pass.get());
        if (!target->create()) throw std::runtime_error("Offscreen target creation failed");
        SpikeScene scene;
        readback.completed = [&] { completed = true; };
        OffscreenFrame frame(rhi.get(), attempt);
        scene.render(rhi.get(), target.get(), frame.cb, options.transparent);
        auto *updates = rhi->nextResourceUpdateBatch();
        updates->readBackTexture(QRhiReadbackDescription(texture.get()), &readback);
        frame.cb->resourceUpdate(updates);
        attempt.actualSubmit = frame.end();
        attempt.completed = completed;
        // Do not reset a real driver or abandon live readback storage. Only
        // substitute the host-visible status after the real synchronous frame.
        if (attempt.actualSubmit != QRhi::FrameOpSuccess)
            throw std::runtime_error("Actual offscreen submission failed");
        attempt.effectiveSubmit = attempt.fault == "submit" ? QRhi::FrameOpDeviceLost : attempt.actualSubmit;
        if (attempt.effectiveSubmit != QRhi::FrameOpSuccess)
            attempt.fail("submit", "Offscreen submission failed");
        if (!completed)
            throw std::runtime_error("Offscreen submission/readback did not complete");
        const QImage image = normalizedReadback(readback, rhi->isYUpInFramebuffer());
        attempt.accepted = true;
        runtime.insert("elapsed_ns", double(timer.nsecsElapsed()));
        runtime.insert("gpu_time_ns", QJsonValue::Null);
        runtime.insert("timing_scope", "device-create-to-completed-readback; not production latency");
        runtime.insert("uploaded_bytes", double(scene.uploadedBytes()));
        runtime.insert("vertex_count", scene.vertexCount());
        runtime.insert("view", QJsonObject{{"id", "view-" + QString::number(attempt.generation - 1)}, {"dpr", 1},
            {"rendered_frames", 1}, {"submitted_frames", 1}, {"last_submitted_frame", 1},
            {"exposed_rendered_frames", 0}, {"readback_frame", 1}, {"owner_generation", attempt.generation}, {"exposed", false}});
        return {runtime, image}; // All frame/resources/device/instance owners tear down here.
}

int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    initializeSpikeResources();
    try {
        const auto options = spikeOptions();
        if (!options.injectFailure.isEmpty())
            throw std::runtime_error("Terminal failure injection requires the borrowed widget host");
        if (options.scenario == "reparent")
            throw std::runtime_error("Reparent proof requires the borrowed widget host");
        QElapsedTimer timer;
        timer.start();
        QJsonObject runtime;
        QJsonArray views, outputs, generations, faults;
        const QStringList faultPoints{"device-create", "texture-create", "begin-frame", "submit"};
        const bool recovery = options.scenario == "recovery";
        const int count = recovery ? faultPoints.size() : (options.scenario == "lifecycle" ? 3 : 1);
        int creations = 0, destructions = 0;
        for (int generation = 1; generation <= count; ++generation) {
            QJsonObject failed;
            if (recovery) {
                Attempt attempt{generation * 2 - 1};
                attempt.fault = faultPoints[generation - 1];
                bool handled = false;
                try { renderOwned(options, attempt); }
                catch (const InjectedFault &error) { handled = error.point == attempt.fault; }
                if (!handled || attempt.accepted || attempt.creations != attempt.destructions)
                    throw std::runtime_error("Injected failure was not handled/cleaned without accepting pixels");
                qInfo("R0 injected host failure handled: %s", qPrintable(attempt.fault));
                failed = attempt.json();
                creations += attempt.creations;
                destructions += attempt.destructions;
            }
            Attempt attempt{recovery ? generation * 2 : generation};
            auto result = renderOwned(options, attempt);
            if (attempt.creations != 1 || attempt.destructions != 1)
                throw std::runtime_error("Owned device did not tear down before accepting its pixels");
            creations += attempt.creations;
            destructions += attempt.destructions;
            runtime = result.first;
            const QString id = "view-" + QString::number(generation - 1);
            auto view = runtime.take("view").toObject();
            view.insert("id", id);
            views.append(view);
            const QString path = count == 1 ? options.output : options.output + "." + id + ".png";
            outputs.append(spikeImage(result.second, path, id, options.transparent));
            if (recovery) faults.append(QJsonObject{{"fault", faultPoints[generation - 1]}, {"handled", true},
                {"failed_attempt", failed}, {"recovered_attempt", attempt.json()}, {"output_id", id}});
            generations.append(QJsonObject{{"generation", attempt.generation}, {"actual_api", runtime["actual_api"]},
                {"device_name", runtime["device_name"]}, {"device_class", runtime["device_class"]},
                {"readback_completed", true}, {"owner_destroyed", true}});
        }
        runtime.insert("views", views);
        runtime.insert("elapsed_ns_total", double(timer.nsecsElapsed()));
        if (options.scenario == "lifecycle")
            runtime.insert("lifecycle", QJsonObject{{"scope", "owned-QRhi-teardown-recreate"},
                {"owned_rhi_creations", creations}, {"owned_rhi_destructions", destructions}, {"generations", generations}});
        if (recovery)
            runtime.insert("recovery", QJsonObject{{"scope", "injected-host-failures-and-owned-device-recreation"},
                {"injection_method", "host-result-substitution"}, {"physical_device_loss_observed", false},
                {"owned_rhi_creations", creations}, {"owned_rhi_destructions", destructions}, {"faults", faults}});
        writeSpikeReport(options, "offscreen", runtime, outputs);
        return 0;
    } catch (const std::exception &e) {
        spikeFailure(QString::fromUtf8(e.what()));
        return 1;
    }
}
