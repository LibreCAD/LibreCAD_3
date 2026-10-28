#include "scene.h"
#include "support.h"
#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QJsonDocument>
#include <QLabel>
#include <QMenu>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMouseEvent>
#include <QPointer>
#include <QRhiWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>
#include <functional>
#include <array>
#include <stdexcept>
#include <utility>
#include <vector>

#ifdef Q_OS_MACOS
const char *spikeGuiSessionIssue();
#endif

struct LifetimeStats {
    int invalidations = 0, dropped = 0, completedAfterInvalidation = 0, replaced = 0;
    bool pendingAtInvalidation = false, pendingAtDestruction = false, shutdownFailed = false;
    QJsonObject json() const {
        return {{"scope", "widget-consumer-invalidation-and-surface-replacement"},
                {"consumer_invalidations", invalidations}, {"stale_deliveries_dropped", dropped},
                {"gpu_completed_after_invalidation", completedAfterInvalidation}, {"surface_replacements", replaced},
                {"gpu_pending_at_invalidation", pendingAtInvalidation}, {"gpu_pending_at_destruction", pendingAtDestruction},
                {"close_request_stage", "after-readback-recording-before-submission"},
                {"shutdown_failed", shutdownFailed}, {"device_loss_injected", false}};
    }
};

struct Capture {
    QRhiReadbackResult result;
    int generation = 0, deviceGeneration = 0, frame = 0;
    bool complete = false, invalidated = false, explicitClose = false;
};

// CPU storage only. This owner is constructed before QApplication and never
// retains a borrowed device/frame/target. Qt cleanup callbacks precede backend
// destruction, so they cannot be used to release the last pending result owner.
struct FailureProof {
    QString reached;
    QJsonObject runtime, faultView;
    std::array<std::shared_ptr<Capture>, 2> retired;
    int retained = 0, released = 0, actualFinish = -1, effectiveFinish = -1;
    bool pendingAtFault = false, completedBeforeDrain = false, completedAfterDrain = false;
    bool captureCompleteAtRelease = false;
    bool windowsDestroyed = false, qtShutdown = false, expired = true;
    bool reportBlocked = false, imageBlocked = false, zeroExitRequested = false, latchSurvived = false;

    void retain(const std::shared_ptr<Capture> &capture) {
        for (const auto &item : retired) if (item == capture) return;
        // No allocation during a failed widget destructor. More live captures
        // would violate this proof's two-view, terminal-stop invariant.
        if (retained == int(retired.size())) qFatal("Terminal readback retirement exceeded the two-view bound");
        retired[retained++] = capture;
    }
    void afterQtShutdown() {
        qtShutdown = true;
        std::array<std::weak_ptr<Capture>, 2> weak;
        for (int i = 0; i < retained; ++i) {
            weak[i] = retired[i];
            retired[i].reset();
        }
        released = retained;
        for (const auto &item : weak) expired = expired && item.expired();
        if (!expired) spikeFailure("Retired readback still has an owner after Qt shutdown");
    }
    void inject(const QString &point, const SpikeOptions &options, const QJsonObject &info,
                const QJsonObject &view, bool pending) {
        if (!reached.isEmpty()) { spikeFailure("Widget injection executed more than once"); return; }
        reached = point;
        runtime = info;
        faultView = view;
        pendingAtFault = pending;
        spikeFailure("Injected widget failure: " + point);
        // Deliberate R0-only probes: neither successful serialization nor a later
        // zero exit request may erase the terminal result, even during teardown.
        auto blocked = [](auto operation) {
            try { operation(); }
            catch (const std::exception &e) {
                return QString::fromUtf8(e.what()) == "Terminal spike failure forbids successful output";
            }
            return false;
        };
        reportBlocked = blocked([&] { writeSpikeReport(options, "widget", info, {}); });
        QImage rejected(1, 1, QImage::Format_RGBA8888);
        rejected.fill(Qt::transparent);
        imageBlocked = blocked([&] { spikeImage(rejected, options.output, "rejected"); });
        QCoreApplication::exit(0);
        zeroExitRequested = true;
        latchSurvived = spikeHasFailed();
        if (!reportBlocked || !imageBlocked || !latchSurvived)
            spikeFailure("Terminal widget failure allowed successful output/status");
    }
    QJsonObject json(const QString &requested) const {
        return {{"requested", requested}, {"reached", reached}, {"injection_method", "host-terminal-result-substitution"},
            {"physical_device_loss_observed", false}, {"view", faultView}, {"pending_at_injection", pendingAtFault},
            {"actual_finish_result", actualFinish}, {"effective_finish_result", effectiveFinish},
            {"readback_completed_before_drain", completedBeforeDrain}, {"readback_completed_after_drain", completedAfterDrain},
            {"capture_completed_at_release", captureCompleteAtRelease},
            {"retained_results", retained}, {"released_results", released}, {"all_retired_expired", expired},
            {"retirement_release_boundary", "after-QApplication-destruction"},
            {"windows_destroyed", windowsDestroyed}, {"qt_shutdown_complete", qtShutdown},
            {"success_report_blocked", reportBlocked}, {"success_image_blocked", imageBlocked},
            {"zero_exit_requested_after_failure", zeroExitRequested}, {"terminal_latch_survived", latchSurvived}};
    }
};

struct ProofOutput {
    struct Image { QImage pixels; QString path, id; bool transparent; };
    std::vector<Image> images;
    QJsonObject runtime;
    void snapshot(const QImage &image, const QString &path, const QString &id, bool transparent) {
        images.push_back({image, path, id, transparent}); // Retain normalized CPU bytes, not GPU storage.
    }
};

class SpikeWidget final : public QRhiWidget {
public:
    SpikeWidget(const SpikeOptions &options, std::shared_ptr<LifetimeStats> stats, FailureProof &failure,
                bool inject, int ownerGeneration = 1)
        : options(options), transparent(options.transparent), stats(std::move(stats)), failure(failure),
          injectFault(inject), ownerGeneration(ownerGeneration) {
        setApi(options.backend == "metal" ? Api::Metal : Api::Vulkan);
        setSampleCount(1);
        setFocusPolicy(Qt::StrongFocus);
        connect(this, &QRhiWidget::renderFailed, this, [] { spikeFailure("Qt widget render failed"); });
        connect(this, &QRhiWidget::frameSubmitted, this, [this] {
            if (spikeHasFailed()) return;
            if (window()->windowHandle() && window()->windowHandle()->isExposed() &&
                renderedAfterExposure && rendered > lastSubmitted) {
                ++submitted;
                lastSubmitted = rendered;
            }
            if (closeAction && capture && capture->generation == closeGeneration && lastSubmitted >= capture->frame) {
                auto action = std::move(closeAction);
                closeAction = {};
                const QPointer<SpikeWidget> closing(this);
                QTimer::singleShot(0, window(), [closing, action] {
                    if (closing) action(closing.data());
                    else spikeFailure("Close target disappeared before controlled teardown");
                });
            }
        });
    }
    ~SpikeWidget() override { releaseResources(); }
    bool ready() const {
        return !spikeHasFailed() && active && capture && !capture->invalidated && !image.isNull() && renderTarget() &&
               image.size() == renderTarget()->pixelSize() && submitted > 0 && lastSubmitted >= capture->frame;
    }
    void requestNativeContextMenu() {
        nativeMenuPending = true;
        writeSpikeInputMarker(options, true);
    }
    void advanceReadback() { if (!spikeHasFailed() && active && capture && !capture->complete) update(); }
    void recapture() {
        if (spikeHasFailed()) return;
        image = {};
        if (capture) capture->invalidated = true;
        requestCapture = true;
        update();
    }
    QRhi *borrowedDevice() const { return device; }
    int renderedFrames() const { return rendered; }
    void deferNextDelivery() { holdNextDelivery = true; }
    bool hasHeldDelivery() const { return bool(heldDelivery); }
    bool heldCaptureIsCurrent() const { return heldCapture && heldCapture == capture && heldCapture->complete && !heldCapture->invalidated; }
    int droppedRecaptures() const { return invalidatedDeliveries; }
    void deliverHeldCapture() {
        auto delivery = std::move(heldDelivery);
        heldDelivery = {};
        heldCapture.reset();
        if (!delivery) throw std::runtime_error("No completed CPU readback delivery was held");
        delivery();
    }
    void closeAfterNextSubmission(std::function<void(SpikeWidget *)> action) {
        closeGeneration = captureGeneration + 1;
        closeAction = std::move(action);
        recapture();
    }
    bool triggerVisibleMenu() {
        if (!menu || !menu->isVisible() || QApplication::activePopupWidget() != menu.get() ||
            !menu->windowHandle() || !menu->windowHandle()->isExposed()) return false;
        popupObserved = true;
        // Keep the compositor's implicit keyboard grab alive until the popup
        // is actually exposed, not merely queued by the native key callback.
        if (nativePopupSeed) writeSpikeInputMarker(options);
        menu->actions().first()->trigger();
        menu->close();
        return true;
    }
    QJsonObject interactions() const {
        return {{"mouse_presses", mousePresses}, {"space_keys", spaceKeys}, {"context_requests", contextRequests},
                {"menu_actions", menuActions}, {"popup_observed", popupObserved}, {"native_popup_seed", nativePopupSeed}};
    }
    QJsonObject view(const QString &id) const {
        return {{"id", id}, {"dpr", devicePixelRatioF()}, {"exposed", renderedAfterExposure},
                {"rendered_frames", rendered}, {"submitted_frames", submitted}, {"last_submitted_frame", lastSubmitted},
                {"exposed_rendered_frames", exposedRendered}, {"readback_frame", capture ? capture->frame : 0},
                {"owner_generation", ownerGeneration}, {"device_generation", deviceGeneration},
                {"resource_releases", resourceReleases}, {"capture_generation", captureGeneration},
                {"readback_device_generation", capture ? capture->deviceGeneration : 0}, {"device_info", info}};
    }
    QImage image;
    QJsonObject info;
private:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) { ++mousePresses; setFocus(); event->accept(); }
        else QRhiWidget::mousePressEvent(event);
    }
    void keyPressEvent(QKeyEvent *event) override {
        if (event->spontaneous() && event->key() == Qt::Key_F12) {
            if (nativeMenuPending) {
                nativeMenuPending = false;
                nativePopupSeed = true;
                const QPoint local(30, 30);
                QContextMenuEvent menuEvent(QContextMenuEvent::Keyboard, local, mapToGlobal(local));
                QApplication::sendEvent(this, &menuEvent);
            }
            event->accept();
        }
        else if (event->key() == Qt::Key_Space) { ++spaceKeys; event->accept(); }
        else QRhiWidget::keyPressEvent(event);
    }
    void contextMenuEvent(QContextMenuEvent *event) override {
        ++contextRequests;
        menu = std::make_unique<QMenu>(this);
        auto *action = menu->addAction("R0 view action");
        connect(action, &QAction::triggered, this, [this] { ++menuActions; });
        menu->popup(event->globalPos());
        event->accept();
    }
    void releaseResources() override {
        active = false;
        image = {};
        requestCapture = true;
        if (capture) {
            capture->invalidated = true;
            if (capture->explicitClose && !capture->complete) stats->pendingAtDestruction = true;
            const auto lease = capture; // Own storage before finish can invoke its callback.
            const bool shutdownFault = injectFault && options.injectFailure == "shutdown" && failure.reached.isEmpty();
            if (device && (!lease->complete || shutdownFault)) {
                const bool before = lease->complete;
                const auto actual = device->finish();
                const auto effective = shutdownFault && actual == QRhi::FrameOpSuccess ? QRhi::FrameOpError : actual;
                if (injectFault) {
                    failure.actualFinish = actual;
                    failure.effectiveFinish = effective;
                    failure.completedBeforeDrain = before;
                    failure.completedAfterDrain = lease->complete;
                }
                if (effective != QRhi::FrameOpSuccess || !lease->complete) {
                    failure.retain(lease);
                    stats->shutdownFailed = true;
                    if (shutdownFault && actual == QRhi::FrameOpSuccess && lease->complete)
                        failure.inject("shutdown", options, info, view("fault-view"), !before);
                    else spikeFailure("Cannot drain pending widget readback during shutdown");
                }
            } else if (!device && !lease->complete) {
                failure.retain(lease);
                spikeFailure("Pending readback has no live device during release");
            }
            if (injectFault) failure.captureCompleteAtRelease = lease->complete;
        }
        if (scene) { scene.reset(); ++resourceReleases; }
        device = nullptr;
    }
    void initialize(QRhiCommandBuffer *) override {
        if (spikeHasFailed()) return;
        if (device && device != rhi()) {
            spikeFailure("Qt changed the borrowed device without releasing its scene resources");
            return;
        }
        if (device != rhi()) {
            device = rhi();
            scene = std::make_unique<SpikeScene>();
            active = true;
            renderedAfterExposure = false;
            ++deviceGeneration;
        } else recapture(); // Target resize/format changes invalidate the completed CPU snapshot too.
    }
    void render(QRhiCommandBuffer *cb) override {
        if (spikeHasFailed() || !active || !isVisible() || width() <= 0 || height() <= 0 || !renderTarget() || renderTarget()->pixelSize().isEmpty())
            return;
        try {
            const bool exposed = window()->windowHandle() && window()->windowHandle()->isExposed();
            renderedAfterExposure = renderedAfterExposure || exposed;
            info = spikeDeviceInfo(rhi());
            requireSpikeCapabilities(rhi(), renderTarget()->pixelSize());
            if (injectFault && options.injectFailure == "render" && exposed) {
                failure.inject("render", options, info, view("fault-view"), false);
                return;
            }
            scene->render(rhi(), renderTarget(), cb, transparent);
            ++rendered;
            if (exposed) ++exposedRendered;
            if (exposed && requestCapture && (!capture || capture->complete)) {
                requestCapture = false;
                capture = std::make_shared<Capture>();
                capture->generation = ++captureGeneration;
                capture->deviceGeneration = deviceGeneration;
                capture->frame = rendered;
                const bool yUp = rhi()->isYUpInFramebuffer();
                const QPointer<SpikeWidget> guard(this);
                const std::weak_ptr<Capture> weak = capture;
                const auto counters = stats;
                capture->result.completed = [weak, guard, yUp, counters] {
                    const auto result = weak.lock();
                    if (!result) { spikeFailure("Readback owner expired before GPU completion"); return; }
                    result->complete = true;
                    if (result->explicitClose) ++counters->completedAfterInvalidation;
                    try {
                        const auto pixels = normalizedReadback(result->result, yUp);
                        auto delivery = [result, guard, pixels, counters] {
                            if (spikeHasFailed() || result->invalidated || !guard || !guard->active ||
                                guard->captureGeneration != result->generation) {
                                if (result->explicitClose) ++counters->dropped;
                                else if (guard) ++guard->invalidatedDeliveries;
                                return;
                            }
                            guard->image = pixels;
                        };
                        if (spikeHasFailed() || !guard || !QCoreApplication::instance()) delivery();
                        else if (guard->holdNextDelivery) {
                            guard->holdNextDelivery = false;
                            guard->heldCapture = result;
                            guard->heldDelivery = std::move(delivery);
                        } else QTimer::singleShot(0, QCoreApplication::instance(), std::move(delivery));
                    } catch (const std::exception &e) { spikeFailure(QString::fromUtf8(e.what())); }
                };
                auto *updates = rhi()->nextResourceUpdateBatch();
                updates->readBackTexture(QRhiReadbackDescription(colorTexture()), &capture->result);
                cb->resourceUpdate(updates);
                if (injectFault && options.injectFailure == "readback") {
                    capture->invalidated = true;
                    active = false;
                    failure.inject("readback", options, info, view("fault-view"), !capture->complete);
                    return;
                }
                if (closeAction && capture->generation == closeGeneration) {
                    // Logical close can be requested while work is recorded. Do
                    // not destroy the widget or any borrowed frame objects here.
                    active = false;
                    capture->invalidated = capture->explicitClose = true;
                    ++stats->invalidations;
                    stats->pendingAtInvalidation = !capture->complete;
                }
            }
        } catch (const std::exception &e) { spikeFailure(QString::fromUtf8(e.what())); }
    }
    SpikeOptions options;
    bool transparent, active = true;
    std::shared_ptr<LifetimeStats> stats;
    FailureProof &failure;
    bool injectFault;
    std::shared_ptr<Capture> capture, heldCapture;
    std::unique_ptr<SpikeScene> scene;
    std::unique_ptr<QMenu> menu;
    QRhi *device = nullptr;
    std::function<void(SpikeWidget *)> closeAction;
    std::function<void()> heldDelivery;
    int ownerGeneration, captureGeneration = 0, closeGeneration = 0, deviceGeneration = 0, resourceReleases = 0;
    int rendered = 0, submitted = 0, lastSubmitted = 0, exposedRendered = 0;
    int mousePresses = 0, spaceKeys = 0, contextRequests = 0, menuActions = 0;
    bool renderedAfterExposure = false, requestCapture = true, popupObserved = false, holdNextDelivery = false;
    bool nativePopupSeed = false, nativeMenuPending = false;
    int invalidatedDeliveries = 0;
};

static int runWidgetProof(QApplication &app, const SpikeOptions &options, FailureProof &failure, ProofOutput &output) {
    try {
        if (options.scenario == "recovery")
            throw std::runtime_error("Recovery proof requires the owned offscreen host");
        if (QGuiApplication::platformName() == "offscreen" || QGuiApplication::platformName() == "minimal")
            throw std::runtime_error("Widget proof requires a real presentation QPA/session");
#ifdef Q_OS_MACOS
        if (const auto issue = spikeGuiSessionIssue()) throw std::runtime_error(issue);
#endif
        QWidget window;
        window.setWindowTitle("LibreCAD R0 RHI proof");
        auto *layout = new QVBoxLayout(&window);
        auto *label = new QLabel("RHI + ordinary widget composition");
        label->setStyleSheet("background: rgb(192,32,64); color: white; padding: 8px");
        layout->addWidget(label);
        auto *mdi = new QMdiArea;
        layout->addWidget(mdi);
        const auto stats = std::make_shared<LifetimeStats>();
        SpikeWidget *views[2];
        QMdiSubWindow *subwindows[2];
        auto addView = [&](int index, int generation) {
            views[index] = new SpikeWidget(options, stats, failure, index == 0, generation);
            subwindows[index] = mdi->addSubWindow(views[index]);
            subwindows[index]->setWindowTitle("CAD view");
            views[index]->show();
        };
        addView(0, 1);
        addView(1, 1);
        window.resize(720, 420);
        window.show();
        mdi->tileSubWindows();
        QWidget detachedWindow;
        detachedWindow.setWindowTitle("LibreCAD R0 reparent destination");
        auto *detachedLayout = new QVBoxLayout(&detachedWindow);
        enum class Phase { Initial, Resized, Focus, Menu, Replacement, RecaptureHeld, Hidden, ZeroSize, Resumed, Detached, Returned, Done };
        Phase phase = Phase::Initial;
        QSize before[2];
        QElapsedTimer focusWait;
        QJsonObject interactionProof;
        QElapsedTimer suspendWait;
        QElapsedTimer quiescentWait;
        int quiescentFrames = -1;
        int recaptureDrops = 0;
        bool completedBeforeInvalidation = false;
        QSize previousMinimum, previousMaximum;
        QRhi *originalDevice = nullptr; // Compared only while the surviving view keeps it alive.
        QJsonArray historyViews, stages, suspensions;
        auto snapshot = [&](const QString &name) {
            if (!originalDevice) originalDevice = views[1]->borrowedDevice();
            if (!originalDevice) throw std::runtime_error("No live survivor device for reparent proof");
            QJsonArray ids;
            for (auto *view : views) {
                const QString id = "view-" + QString::number(historyViews.size());
                ids.append(id);
                historyViews.append(view->view(id));
                output.snapshot(view->image, options.output + "." + id + ".png", id, options.transparent);
            }
            stages.append(QJsonObject{{"name", name}, {"view_ids", ids},
                {"shared_device", views[0]->borrowedDevice() == views[1]->borrowedDevice()},
                {"survivor_device_unchanged", views[1]->borrowedDevice() == originalDevice},
                {"moving_in_primary_window", views[0]->window() == &window}});
        };
        QTimer poll;
        QObject::connect(&poll, &QTimer::timeout, &window, [&] {
            if (spikeHasFailed()) { app.exit(1); return; }
            for (auto *view : views) if (view) view->advanceReadback();
            if (phase == Phase::Done) return;
            try {
                if (phase == Phase::RecaptureHeld) {
                    if (!views[0]->hasHeldDelivery()) return;
                    completedBeforeInvalidation = views[0]->heldCaptureIsCurrent();
                    if (!completedBeforeInvalidation)
                        throw std::runtime_error("Held readback was not current/completed before recapture invalidation");
                    const int beforeDrops = views[0]->droppedRecaptures();
                    views[0]->recapture();
                    views[0]->deliverHeldCapture();
                    recaptureDrops = views[0]->droppedRecaptures() - beforeDrops;
                    if (recaptureDrops != 1 || !views[0]->image.isNull() || views[0]->ready())
                        throw std::runtime_error("Invalidated recapture delivery was accepted");
                    previousMinimum = views[0]->minimumSize();
                    previousMaximum = views[0]->maximumSize();
                    views[0]->hide();
                    suspendWait.start();
                    phase = Phase::Hidden;
                    return;
                }
                if (phase == Phase::Hidden || phase == Phase::ZeroSize) {
                    const bool hidden = phase == Phase::Hidden;
                    if ((hidden && views[0]->isVisible()) || (!hidden && views[0]->size() != QSize(0, 0)))
                        throw std::runtime_error("Suspend prerequisite was not observed");
                    // Allow event-queue settling, then require a stable no-draw interval.
                    if (suspendWait.elapsed() < 100) return;
                    if (quiescentFrames < 0) {
                        quiescentFrames = views[0]->renderedFrames();
                        quiescentWait.start();
                    }
                    if (views[0]->renderedFrames() != quiescentFrames)
                        throw std::runtime_error("Hidden/zero-sized scene continued drawing");
                    if (quiescentWait.elapsed() < 200) return;
                    suspensions.append(QJsonObject{{"kind", hidden ? "hidden" : "zero-size"},
                        {"observed", true}, {"observed_ms", double(quiescentWait.elapsed())},
                        {"visible", views[0]->isVisible()}, {"logical_width", views[0]->width()},
                        {"logical_height", views[0]->height()}, {"rendered_delta", views[0]->renderedFrames() - quiescentFrames}});
                    if (hidden) {
                        views[0]->setFixedSize(0, 0);
                        views[0]->show();
                        suspendWait.restart();
                        quiescentFrames = -1;
                        phase = Phase::ZeroSize;
                    } else {
                        views[0]->setMinimumSize(previousMinimum);
                        views[0]->setMaximumSize(previousMaximum);
                        mdi->tileSubWindows();
                        for (auto *view : views) view->recapture();
                        phase = Phase::Resumed;
                    }
                    return;
                }
                if (phase == Phase::Focus) {
                    if (!views[0]->hasFocus() || QApplication::focusWidget() != views[0]) {
                        if (focusWait.elapsed() > 3000)
                            throw std::runtime_error(("MDI keyboard focus was not acquired; app_state=" +
                                QString::number(QGuiApplication::applicationState()) + "; active_window=" +
                                QString::number(window.isActiveWindow()) + "; focus_window=" +
                                (QGuiApplication::focusWindow() ? QGuiApplication::focusWindow()->title() : QString("none")) +
                                "; focus_widget=" + (QApplication::focusWidget() ?
                                    QString::fromLatin1(QApplication::focusWidget()->metaObject()->className()) : QString("none")) +
                                "; window_flags=" + QString::number(quint64(window.windowFlags()))).toStdString());
                        window.raise();
                        window.windowHandle()->requestActivate();
                        mdi->setActiveSubWindow(subwindows[0]);
                        views[0]->setFocus();
                        return;
                    }
                    const QPointF local(30, 30), global(views[0]->mapToGlobal(QPoint(30, 30)));
                    QMouseEvent mouse(QEvent::MouseButtonPress, local, local, global,
                                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                    QApplication::sendEvent(views[0], &mouse);
                    QMouseEvent release(QEvent::MouseButtonRelease, local, local, global,
                                        Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
                    QApplication::sendEvent(views[0], &release);
                    QKeyEvent key(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
                    if (QApplication::focusWidget() != views[0])
                        throw std::runtime_error("Mouse handling changed the intended keyboard recipient");
                    QApplication::sendEvent(QApplication::focusWidget(), &key);
                    if (QGuiApplication::platformName() == "wayland") {
                        // Request the popup in the native key callback, while
                        // that Wayland seat/serial is current, not a later timer.
                        views[0]->requestNativeContextMenu();
                    } else {
                        QContextMenuEvent menu(QContextMenuEvent::Mouse, QPoint(30, 30), global.toPoint());
                        QApplication::sendEvent(views[0], &menu);
                    }
                    phase = Phase::Menu;
                    return;
                }
                if (phase == Phase::Menu) {
                    if (!views[0]->triggerVisibleMenu()) return;
                    interactionProof = {{"dispatch", "synthetic-Qt-events-with-observed-focus"}, {"focused_view", "view-0"},
                        {"keyboard_recipient", "view-0"}, {"focus_observed", true},
                        {"popup_dispatch", QGuiApplication::platformName() == "wayland" ? "native-F12-key-handler" : "synthetic-Qt-context-event"},
                        {"target", views[0]->interactions()}, {"other_view", views[1]->interactions()}};
                    phase = Phase::Replacement;
                    views[0]->closeAfterNextSubmission([&](SpikeWidget *) {
                        views[0] = nullptr;
                        delete subwindows[0]; // Outside render/submission callbacks; drains pending GPU data if needed.
                        ++stats->replaced;
                        addView(0, 2);
                        mdi->tileSubWindows();
                        views[1]->recapture();
                    });
                    return;
                }
                if (phase == Phase::Replacement && (!stats->replaced || !stats->dropped)) return;
                if (!views[0] || !views[1] || !views[0]->ready() || !views[1]->ready()) return;
                if (phase == Phase::Initial) {
                    for (int i = 0; i < 2; ++i) before[i] = views[i]->image.size();
                    window.resize(820, 500);
                    mdi->tileSubWindows();
                    for (auto *view : views) view->recapture();
                    phase = Phase::Resized;
                    return;
                }
                if (phase == Phase::Resized)
                    for (int i = 0; i < 2; ++i) if (views[i]->image.size() == before[i]) return;
                if (options.scenario == "reparent") {
                    if (phase == Phase::Resized) {
                        snapshot("initial");
                        views[0]->deferNextDelivery();
                        views[0]->recapture();
                        phase = Phase::RecaptureHeld;
                        return;
                    }
                    if (phase == Phase::Resumed) {
                        snapshot("resumed");
                        // setWidget(nullptr) relinquishes, rather than deletes, the old internal widget.
                        subwindows[0]->setWidget(nullptr);
                        subwindows[0]->hide();
                        detachedLayout->addWidget(views[0]);
                        detachedWindow.resize(460, 340);
                        detachedWindow.show();
                        views[0]->show();
                        for (auto *view : views) view->recapture();
                        phase = Phase::Detached;
                        return;
                    }
                    if (phase == Phase::Detached) {
                        snapshot("detached");
                        detachedLayout->removeWidget(views[0]);
                        subwindows[0]->setWidget(views[0]);
                        subwindows[0]->show();
                        views[0]->show();
                        detachedWindow.hide();
                        mdi->tileSubWindows();
                        for (auto *view : views) view->recapture();
                        phase = Phase::Returned;
                        return;
                    }
                    if (phase != Phase::Returned) throw std::runtime_error("Unexpected reparent proof phase");
                    snapshot("returned");
                }
                if (phase == Phase::Resized && options.scenario == "lifecycle") {
                    mdi->setActiveSubWindow(subwindows[0]);
                    window.raise();
                    window.activateWindow();
                    views[0]->setFocus();
                    focusWait.start();
                    phase = Phase::Focus;
                    return;
                }
                QJsonArray descriptions;
                for (int i = 0; i < 2 && options.scenario != "reparent"; ++i) {
                    const QString id = "view-" + QString::number(i);
                    descriptions.append(views[i]->view(id));
                    output.snapshot(views[i]->image, options.output + "." + id + ".png", id, options.transparent);
                }
                auto runtime = views[0]->info;
                if (options.scenario == "reparent") {
                    descriptions = historyViews;
                    runtime.insert("reparent", QJsonObject{{"scope", "Qt-borrowed-device-cross-window-reparent"},
                        {"stages", stages}, {"suspensions", suspensions}, {"physical_device_loss_observed", false},
                        {"shutdown_failed", stats->shutdownFailed}, {"recapture_delivery_drops", recaptureDrops},
                        {"recapture_completed_before_invalidation", completedBeforeInvalidation}});
                }
                if (views[1]->info["actual_api"] != runtime["actual_api"])
                    throw std::runtime_error("MDI views disagree on actual backend");
                runtime.insert("views", descriptions);
                runtime.insert("resized", true);
                if (options.scenario == "lifecycle") {
                    runtime.insert("interactions", interactionProof);
                    runtime.insert("lifecycle", stats->json());
                }
                output.snapshot(window.grab().toImage(), options.output + ".composition.png", "composition", false);
                output.runtime = std::move(runtime);
                phase = Phase::Done;
                app.exit(0);
            } catch (const std::exception &e) { phase = Phase::Done; spikeFailure(QString::fromUtf8(e.what())); }
        });
        poll.start(50);
        QTimer::singleShot(20000, &window, [&] {
            QJsonObject details{{"phase", int(phase)}, {"lifetime", stats->json()}};
            if (views[0]) details.insert("target", views[0]->view("view-0"));
            if (views[0]) details.insert("input", views[0]->interactions());
            spikeFailure("Timed out waiting for exposed MDI/readback/lifecycle proof: " +
                         QString::fromUtf8(QJsonDocument(details).toJson(QJsonDocument::Compact)));
        });
        if (spikeHasFailed()) return 1; // A synchronous show/paint can fail before exec starts.
        return app.exec();
    } catch (const std::exception &e) { spikeFailure(QString::fromUtf8(e.what())); return 1; }
}

int main(int argc, char **argv) {
    FailureProof failure; // Must outlive QApplication, including failed backend teardown.
    ProofOutput output;
    SpikeOptions options;
    bool parsed = false;
    int status = 1;
    {
        QApplication app(argc, argv);
        initializeSpikeResources();
        try {
            options = spikeOptions();
            parsed = true;
            status = runWidgetProof(app, options, failure, output);
        } catch (const std::exception &e) { spikeFailure(QString::fromUtf8(e.what())); }
        failure.windowsDestroyed = true;
    }
    failure.afterQtShutdown();
    try {
        if (parsed && !options.injectFailure.isEmpty() && failure.reached.isEmpty())
            spikeFailure("Requested widget failure injection was not reached");
        if (status != 0 && !spikeHasFailed()) spikeFailure("Widget event loop returned a failure status");
        if (spikeHasFailed()) {
            if (parsed && !failure.reached.isEmpty())
                writeSpikeFailureReport(options, failure.runtime, failure.json(options.injectFailure));
            return 1;
        }
        if (output.runtime.isEmpty()) throw std::runtime_error("Widget proof has no completed runtime result");
        QJsonArray images;
        for (const auto &image : output.images)
            images.append(spikeImage(image.pixels, image.path, image.id, image.transparent));
        writeSpikeReport(options, "widget", output.runtime, images);
        return 0;
    } catch (const std::exception &e) { spikeFailure(QString::fromUtf8(e.what())); return 1; }
}
