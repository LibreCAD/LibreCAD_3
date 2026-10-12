// Phase 5 PR-5.6 — runtime smoke tests for the full lcgui binding surface.
//
// These tests exist BECAUSE the coordinator called out a pattern: "a
// plausible-looking binding call that turns out to reference a signature
// that doesn't actually exist" — every session blocker so far
// (`_get_main_window`, `addCommand(std::string, ...)`, `EntityBuilder(doc)`,
// `Line(a, b, layer)`) was caught by real runtime invocation, not code
// review.  This file exercises each newly-bound class by CONSTRUCTING an
// instance and CALLING at least one method against real widget state,
// closing the code-review-alone gap for PR-5.6's binding volume.
//
// Behind #ifdef LC_WITH_PYTHONSCRIPT so an OFF build (Windows/embed
// CPython) doesn't compile-fail here.  Skipped entirely otherwise —
// same pattern as testscriptdock.cpp.

#include <QApplication>
#include <gtest/gtest.h>
#include <memory>
#include <new>
#include <type_traits>
#include <thread>
#include <iostream>

#include <mainwindow.h>
#include <cadmdichild.h>
#include <windowmanager.h>
#include <widgets/guiAPI/inputguicontainer.h>
#include <widgets/guiAPI/textgui.h>
#include <widgets/guiAPI/menuitem.h>
#include <widgets/guiAPI/toolbarbutton.h>
#include <widgets/guiAPI/menu.h>
#include <widgets/guiAPI/dialogwidget.h>
#include <widgets/guiAPI/colorgui.h>
#include <widgets/guiAPI/entitygui.h>
#include <QListWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QElapsedTimer>
#include <QTemporaryDir>
#include <QFile>
#include <QLineEdit>
#include <QScopeGuard>
#include <QOpenGLContext>
#include <widgets/layermodel.h>
#include <cad/primitive/point.h>
#include <widgets/clicommand.h>
#include <luainterface.h>
#include <drawables/tempentities.h>

#ifdef LC_WITH_PYTHONSCRIPT
#include <qt_keywords_push.h>
#include <pybind11/embed.h>
#include <qt_keywords_pop.h>
#include <lcpython.h>
#include <python/pyeventhooks.h>
#include <python/qtpythonlifetime.h>
#include <scriptadapter/pythoncallback.h>
#endif

#include "../uitests.h"

#ifdef LC_WITH_PYTHONSCRIPT

// Small RAII helper: register a MainWindow with WindowManager so
// lcgui.currentMainWindow() finds it, then remove on destruction.  The
// MainWindow constructor doesn't self-register — WindowManager::init /
// newFile / openFile normally does that; unit tests must push
// explicitly.  Same pattern as testscriptdock.cpp's
// PythonEventDrivenOnEventReachesMainWindow.
namespace {

struct MainWindowRegistration {
    lc::ui::MainWindow* mw;
    explicit MainWindowRegistration(lc::ui::MainWindow* window) : mw(window) {
        lc::ui::WindowManager::mainWindows.push_back(mw);
    }
    ~MainWindowRegistration() {
        auto& v = lc::ui::WindowManager::mainWindows;
        v.erase(std::remove(v.begin(), v.end(), mw), v.end());
    }
};

struct TempEntityRefreshCounter {
    int refreshCount = 0;

    void onRefresh() {
        ++refreshCount;
    }
};

// Drive the actual modal file dialog, with a deadline and no native OS panel.
struct OpenDialogDriver {
    QTimer timer;
    QElapsedTimer elapsed;
    bool previous = QApplication::testAttribute(Qt::AA_DontUseNativeDialogs);
    bool selected = false;
    bool timedOut = false;
    int messages = 0;
    explicit OpenDialogDriver(QString path = {}) {
        QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, true);
        elapsed.start();
        QObject::connect(&timer, &QTimer::timeout, [this, path] {
            auto* modal = QApplication::activeModalWidget();
            if (auto* dialog = qobject_cast<QFileDialog*>(modal)) {
                selected = true;
                if (path.isEmpty()) dialog->reject();
                else {
                    // Enter the absolute path through the real filename field;
                    // selectFile alone races the asynchronous directory model.
                    auto* edit = dialog->findChild<QLineEdit*>("fileNameEdit");
                    if (edit) edit->setText(path);
                    QMetaObject::invokeMethod(dialog, "accept", Qt::DirectConnection);
                }
            } else if (auto* message = qobject_cast<QMessageBox*>(modal)) {
                ++messages;
                message->accept();
            }
            if (elapsed.elapsed() > 5000) {
                timedOut = true;
                if (auto* dialog = qobject_cast<QDialog*>(modal)) dialog->reject();
            }
        });
        timer.start(10);
    }
    ~OpenDialogDriver() {
        timer.stop();
        QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, previous);
    }
};

} // namespace

TEST(PyGuiBridgeTest, DocumentReplacementCancelsOperationAndIsolatesStorage) {
    ensureUiApplication();
    lc::ui::MainWindow first;
    lc::ui::MainWindow second;
    first.layers()->addLayer("old-layer");
    first.cliCommand()->runCommand("PYRECTANGLE");
    first.triggerCoordinateEntered(lc::geo::Coordinate(1, 2));
    first.triggerCoordinateEntered(lc::geo::Coordinate(11, 10));
    auto oldDocument = first.cadMdiChild()->document();
    auto oldStorage = first.cadMdiChild()->storageManager();
    auto oldUndo = first.cadMdiChild()->undoManager();
    ASSERT_EQ(oldDocument->entityContainer().asVector().size(), 1u);
    first.cliCommand()->runCommand("PYRECTANGLE");
    first.triggerCoordinateEntered(lc::geo::Coordinate(3, 4));
    auto previewPayload = lc::scripting::makeMap();
    (*previewPayload)["position"] = lc::geo::Coordinate(9, 9);
    first.luaInterface()->triggerEvent("mouseMove", lc::scripting::ScriptValue(previewPayload));
    second.cliCommand()->runCommand("PYRECTANGLE");
    second.triggerCoordinateEntered(lc::geo::Coordinate(20, 30));
    auto oldOperation = first.currentOperation();
    ASSERT_FALSE(oldOperation.isNil());
    TempEntityRefreshCounter preview;
    auto oldPreview = first.cadMdiChild()->tempEntities();
    oldPreview->requestUpdateEvent().connect<TempEntityRefreshCounter,
        &TempEntityRefreshCounter::onRefresh>(preview);
    {
        lc::ui::python::MainWindowExecutionScope outer(&second);
        pybind11::gil_scoped_acquire gil;
        pybind11::dict ns;
        ns["a"] = pybind11::cast(&first, pybind11::return_value_policy::reference);
        pybind11::exec("a.cadMdiChild().newDocument()", ns);
        EXPECT_EQ(lc::ui::python::currentMainWindow(), &second);
    }
    EXPECT_TRUE(first.currentOperation().isNil());
    EXPECT_TRUE(oldOperation.getAttr("finished").asBool());
    EXPECT_EQ(first.luaInterface()->listenerCount("point"), 0u);
    EXPECT_EQ(first.luaInterface()->listenerCount("mouseMove"), 0u);
    EXPECT_EQ(second.luaInterface()->listenerCount("point"), 1u);
    EXPECT_FALSE(second.currentOperation().isNil());
    EXPECT_FALSE(first.cadMdiChild()->viewer()->operationActive());
    EXPECT_GT(preview.refreshCount, 0) << "close must remove the old preview before replacement";
    EXPECT_NE(first.cadMdiChild()->storageManager(), oldStorage);
    EXPECT_EQ(first.findChildren<lc::ui::LCADPaperViewer*>().size(), 2);
    EXPECT_EQ(first.findChild<lc::ui::widgets::LayerModel*>()->rowCount({}), 1);
    EXPECT_TRUE(first.cadMdiChild()->document()->entityContainer().asVector().empty());
    EXPECT_EQ(oldDocument->entityContainer().asVector().size(), 1u);
    oldUndo->undo();
    EXPECT_TRUE(oldDocument->entityContainer().asVector().empty());
    EXPECT_TRUE(first.cadMdiChild()->document()->entityContainer().asVector().empty());
    first.cliCommand()->runCommand("PYRECTANGLE");
    first.triggerCoordinateEntered(lc::geo::Coordinate(5, 6));
    first.triggerCoordinateEntered(lc::geo::Coordinate(15, 16));
    EXPECT_EQ(first.cadMdiChild()->document()->entityContainer().asVector().size(), 1u);
    EXPECT_TRUE(oldDocument->entityContainer().asVector().empty());
    EXPECT_EQ(second.luaInterface()->listenerCount("point"), 1u);
    second.triggerFinishOperation();
}

TEST(PyGuiBridgeTest, ActualFileReopenReconstructsInItsWindowAndCancelPreservesDrawing) {
    ensureUiApplication();
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const auto path = directory.path() + QString::fromUtf8("/drawing Ω.dxf");
    lc::ui::MainWindow first;
    lc::ui::MainWindow second;
    first.cliCommand()->runCommand("PYRECTANGLE");
    first.triggerCoordinateEntered(lc::geo::Coordinate(1, 2));
    first.triggerCoordinateEntered(lc::geo::Coordinate(11, 10));
    ASSERT_TRUE(lc::persistence::File::exportFile(first.cadMdiChild()->document(),
        path.toStdString(), lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
    auto originalDocument = first.cadMdiChild()->document();
    int replacements = 0;
    std::string notifiedFilename;
    QObject::connect(first.cadMdiChild(), &lc::ui::CadMdiChild::documentChanged,
                     &first, [&] {
        ++replacements;
        notifiedFilename = first.cadMdiChild()->getFilename();
    });
    first.cliCommand()->runCommand("PYRECTANGLE");
    first.triggerCoordinateEntered(lc::geo::Coordinate(3, 4));
    auto activeOperation = first.currentOperation();
    {
        OpenDialogDriver dialog;
        EXPECT_FALSE(first.cadMdiChild()->openFile());
        EXPECT_TRUE(dialog.selected);
        EXPECT_FALSE(dialog.timedOut);
    }
    EXPECT_EQ(first.cadMdiChild()->document(), originalDocument);
    EXPECT_EQ(first.luaInterface()->listenerCount("point"), 1u);
    QFile malformed(directory.path() + "/broken.dxf");
    ASSERT_TRUE(malformed.open(QIODevice::WriteOnly));
    malformed.write("0\nSECTION\n2\nENTITIES\n0\nLINE\n10\n");
    malformed.close();
    {
        OpenDialogDriver dialog(malformed.fileName());
        EXPECT_FALSE(first.cadMdiChild()->openFile());
        EXPECT_TRUE(dialog.selected);
        EXPECT_FALSE(dialog.timedOut);
        EXPECT_GT(dialog.messages, 0);
    }
    EXPECT_EQ(first.cadMdiChild()->document(), originalDocument);
    EXPECT_EQ(first.luaInterface()->listenerCount("point"), 1u);
    pybind11::gil_scoped_acquire gil;
    EXPECT_EQ(replacements, 0);
    pybind11::dict ns;
    ns["a"] = pybind11::cast(&first, pybind11::return_value_policy::reference);
    ns["is_registered"] = pybind11::cpp_function([](lc::ui::MainWindow* window) {
        const auto& windows = lc::ui::WindowManager::mainWindows;
        return std::find(windows.begin(), windows.end(), window) != windows.end();
    });
    pybind11::exec(R"py(
import lc, lcgui as gui
from lcUIPy.plugins.rectangle import plugin as rectangle
observed = []
def reconstruct(insert):
    owner = gui.currentMainWindow()
    observed.append((owner, insert.document(), is_registered(owner) if owner else None))
    rectangle._on_new_waiting_custom_entity(insert)
lc.register_plugin('LC Plugin', reconstruct)
)py", ns);
    auto restorePlugin = qScopeGuard([&] {
        ns["lc"].attr("register_plugin")("LC Plugin",
            ns["rectangle"].attr("_on_new_waiting_custom_entity"));
    });
    {
        lc::ui::python::MainWindowExecutionScope outer(&second);
        OpenDialogDriver dialog(path);
        pybind11::exec("opened = a.cadMdiChild().openFile()", ns);
        EXPECT_TRUE(ns["opened"].cast<bool>());
        EXPECT_FALSE(dialog.timedOut);
        EXPECT_EQ(lc::ui::python::currentMainWindow(), &second);
    }
    EXPECT_TRUE(activeOperation.getAttr("finished").asBool());
    EXPECT_TRUE(first.currentOperation().isNil());
    EXPECT_EQ(first.luaInterface()->listenerCount("point"), 0u);
    EXPECT_EQ(first.cadMdiChild()->getFilename(), path.toStdString());
    EXPECT_EQ(replacements, 1);
    EXPECT_EQ(notifiedFilename, path.toStdString());
    EXPECT_NO_THROW(pybind11::exec(R"py(
doc = a.cadMdiChild().document()
assert len(observed) == 1 and observed[0][:2] == (a, doc), observed
entities = doc.entityContainer().asVector(32767)
assert len(entities) == 1 and isinstance(entities[0], lc.entity.ScriptCustomEntity)
assert len(rectangle._snap_points(entities[0], lc.geo.Coordinate(), None, 0, 4)) == 4
assert len(doc.entitiesByBlock(entities[0].displayBlock()).asVector(32767)) == 4
)py", ns));
    auto loadedDocument = first.cadMdiChild()->document();
    auto loadedUndo = first.cadMdiChild()->undoManager();
    for (const auto& chosen : {QString(), malformed.fileName()}) {
        OpenDialogDriver dialog(chosen);
        EXPECT_FALSE(first.cadMdiChild()->openFile());
        EXPECT_FALSE(dialog.timedOut);
        EXPECT_EQ(first.cadMdiChild()->document(), loadedDocument);
        EXPECT_EQ(first.cadMdiChild()->undoManager(), loadedUndo);
        EXPECT_EQ(first.cadMdiChild()->getFilename(), path.toStdString());
    }
    const auto before = lc::ui::WindowManager::mainWindows;
    EXPECT_EQ(replacements, 1);
    auto cleanupWindows = qScopeGuard([before] {
        const auto current = lc::ui::WindowManager::mainWindows;
        for (auto* window : current)
            if (std::find(before.begin(), before.end(), window) == before.end()) delete window;
    });
    {
        OpenDialogDriver dialog;
        lc::ui::WindowManager::openFile();
        EXPECT_FALSE(dialog.timedOut);
        EXPECT_EQ(lc::ui::WindowManager::mainWindows.size(), before.size());
    }
    {
        OpenDialogDriver dialog(malformed.fileName());
        lc::ui::WindowManager::openFile();
        EXPECT_FALSE(dialog.timedOut);
        EXPECT_EQ(lc::ui::WindowManager::mainWindows.size(), before.size());
    }
    {
        lc::ui::python::MainWindowExecutionScope outer(&second);
        OpenDialogDriver dialog(path);
        lc::ui::WindowManager::openFile();
        EXPECT_FALSE(dialog.timedOut);
        EXPECT_EQ(lc::ui::python::currentMainWindow(), &second);
    }
    auto* reopened = lc::ui::WindowManager::mainWindows.back();
    ns["reopened"] = pybind11::cast(reopened, pybind11::return_value_policy::reference);
    EXPECT_NO_THROW(pybind11::exec(R"py(
assert observed[-1] == (reopened, reopened.cadMdiChild().document(), False), observed
assert len(reopened.cadMdiChild().document().entityContainer().asVector(32767)) == 1
)py", ns));
    first.cadMdiChild()->newDocument();
    EXPECT_TRUE(first.cadMdiChild()->getFilename().empty());
    EXPECT_EQ(replacements, 2);
    EXPECT_TRUE(notifiedFilename.empty());
    for (auto* window : {&first, reopened}) {
        const auto count = window->cadMdiChild()->document()->entityContainer().asVector().size();
        window->cliCommand()->runCommand("PYRECTANGLE");
        window->triggerCoordinateEntered(lc::geo::Coordinate(30, 40));
        window->triggerCoordinateEntered(lc::geo::Coordinate(40, 50));
        EXPECT_EQ(window->cadMdiChild()->document()->entityContainer().asVector().size(), count + 1);
        window->cadMdiChild()->undoManager()->undo();
        EXPECT_EQ(window->cadMdiChild()->document()->entityContainer().asVector().size(), count);
    }
}

TEST(PyGuiBridgeTest, RetainedDrawingAndPreviewSurviveViewReplacementAndWindowTeardown) {
    ensureUiApplication();
    auto window = std::unique_ptr<lc::ui::MainWindow>(new lc::ui::MainWindow());
    auto document = window->cadMdiChild()->document();
    auto preview = window->cadMdiChild()->tempEntities();
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["document"] = pybind11::cast(document);
    pybind11::exec(R"py(
import lc
line = lc.builder.LineBuilder()
line.setLayer(document.layerByName('0'))
line.setStartPoint(lc.geo.Coordinate(1, 2))
line.setEndPoint(lc.geo.Coordinate(3, 4))
entity = line.build()
)py", ns);
    for (int i = 0; i < 3; ++i) {
        auto papers = window->findChildren<lc::ui::LCADPaperViewer*>();
        ASSERT_EQ(papers.size(), 2);
        QPointer<lc::ui::LCADPaperViewer> active = papers.back();
        ASSERT_TRUE(QMetaObject::invokeMethod(active, "onMouseMoveEvent", Qt::DirectConnection));
        EXPECT_EQ(window->cadMdiChild()->viewer(), active.data());
        window->cadMdiChild()->newDocument();
        EXPECT_TRUE(active.isNull());
        EXPECT_NE(window->cadMdiChild()->viewer(), active.data());
        EXPECT_EQ(window->findChildren<lc::ui::LCADPaperViewer*>().size(), 2);
    }
    auto* layers = window->findChild<lc::ui::widgets::LayerModel*>();
    ASSERT_NE(layers, nullptr);
    EXPECT_EQ(layers->rowCount({}), 1);
    EXPECT_NO_THROW(pybind11::exec(R"py(
old_layer = lc.meta.Layer('retained-only', lc.meta.MetaLineWidthByValue(1),
                          lc.Color(255, 255, 255, 255), None, False)
lc.operation.AddLayer.new(document, old_layer).execute()
)py", ns));
    EXPECT_EQ(layers->rowCount({}), 1) << "old document must not update the new layer model";
    window.reset();
    EXPECT_NO_THROW(pybind11::exec(R"py(
builder = lc.operation.EntityBuilder.new(document)
builder.appendEntity(entity)
builder.execute()
assert len(document.entityContainer().asVector(32767)) == 1
)py", ns));
    preview->addEntity(ns["entity"].cast<lc::entity::CADEntity_CSPtr>());
    preview->removeEntity(ns["entity"].cast<lc::entity::CADEntity_CSPtr>());
}

TEST(PyGuiBridgeTest, UnpaintedCanvasRejectsPointSelectionWithoutLosingDocumentSelection) {
    ensureUiApplication();
    lc::ui::MainWindow window;
    window.cliCommand()->runCommand("PYPOINT");
    window.triggerCoordinateEntered(lc::geo::Coordinate(4, 7));
    auto canvas = window.cadMdiChild()->viewer()->docCanvas();
    ASSERT_EQ(canvas->document()->entityContainer().asVector().size(), 1u);
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["window"] = pybind11::cast(&window, pybind11::return_value_policy::reference);
    EXPECT_NO_THROW(pybind11::exec(R"py(
canvas = window.cadMdiChild().viewer().docCanvas()
canvas.selectAll()
canvas.inverseSelection()
canvas.selectAll()
canvas.removeSelection()
try:
    canvas.selectPoint(4, 7)
except RuntimeError as error:
    assert 'painter' in str(error).lower(), str(error)
else:
    raise AssertionError('unpainted canvas borrowed an absent painter')
)py", ns));
    EXPECT_TRUE(canvas->selectedEntities().asVector().empty());
}

TEST(PyGuiBridgeTest, MaintainedOperationsRestartAfterReplacementAndActualReopen) {
    ensureUiApplication();
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const auto path = directory.path() + QString::fromUtf8("/operations Ω.dxf");
    lc::ui::MainWindow first;
    lc::ui::MainWindow second;
    first.cliCommand()->runCommand("PYPOINT");
    first.triggerCoordinateEntered(lc::geo::Coordinate(-4, -7));
    ASSERT_EQ(first.cadMdiChild()->document()->entityContainer().asVector().size(), 1u);
    ASSERT_TRUE(lc::persistence::File::exportFile(first.cadMdiChild()->document(),
        path.toStdString(), lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
    second.cliCommand()->runCommand("PYGEAR");
    second.triggerNumberEntered(8);
    const auto secondOperation = second.currentOperation();
    auto noListeners = [&] {
        for (const char* event : {"point", "mouseMove", "number", "text"})
            EXPECT_EQ(first.luaInterface()->listenerCount(event), 0u) << event;
    };
    lc::ui::python::MainWindowExecutionScope outer(&second);
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["a"] = pybind11::cast(&first, pybind11::return_value_policy::reference);
    for (bool reopen : {false, true}) {
        SCOPED_TRACE(reopen ? "actual Unicode reopen" : "new document");
        first.cliCommand()->runCommand("PYGEAR");
        first.triggerNumberEntered(8);
        auto canceledGear = first.currentOperation();
        ASSERT_FALSE(canceledGear.isNil());
        auto oldDocument = first.cadMdiChild()->document();
        const auto oldCount = oldDocument->entityContainer().asVector().size();
        if (reopen) {
            OpenDialogDriver dialog(path);
            pybind11::exec("assert a.cadMdiChild().openFile()", ns);
            EXPECT_TRUE(dialog.selected);
            EXPECT_FALSE(dialog.timedOut);
        } else {
            pybind11::exec("a.cadMdiChild().newDocument()", ns);
        }
        EXPECT_TRUE(canceledGear.getAttr("finished").asBool());
        noListeners();
        auto document = first.cadMdiChild()->document();
        auto undo = first.cadMdiChild()->undoManager();
        const std::size_t baseline = reopen ? 1u : 0u;
        ASSERT_EQ(document->entityContainer().asVector().size(), baseline);
        first.clearUndoableStack();
        first.cliCommand()->runCommand("PYGEAR");
        for (double number : {8.0, 20.0, 45.0}) first.triggerNumberEntered(number);
        first.triggerCoordinateEntered(lc::geo::Coordinate(10, 20));
        first.triggerCoordinateEntered(lc::geo::Coordinate(30, 20));
        EXPECT_EQ(document->entityContainer().asVector().size(), baseline + 64);
        EXPECT_NO_THROW(pybind11::exec(R"py(
import lc, math
lines = [e for e in a.cadMdiChild().document().entityContainer().asVector(32767)
         if isinstance(e, lc.entity.Line)]
assert len(lines) == 64, len(lines)
radii = [math.hypot(p.x() - 10, p.y() - 20) for e in lines for p in (e.start(), e.end())]
assert abs(min(radii) - 15) < 1e-8 and abs(max(radii) - 25) < 1e-8, radii
)py", ns));
        noListeners();
        undo->undo();
        EXPECT_EQ(document->entityContainer().asVector().size(), baseline);
        EXPECT_FALSE(undo->canUndo());
        undo->redo();
        EXPECT_EQ(document->entityContainer().asVector().size(), baseline + 64);
        undo->undo();

        first.cliCommand()->runCommand("PYPOINT");
        auto canceledPoint = first.currentOperation();
        auto preview = lc::scripting::makeMap();
        (*preview)["position"] = lc::geo::Coordinate(14, 17);
        first.luaInterface()->triggerEvent("mouseMove", lc::scripting::ScriptValue(preview));
        TempEntityRefreshCounter counter;
        auto temp = first.cadMdiChild()->tempEntities();
        temp->requestUpdateEvent().connect<TempEntityRefreshCounter,
            &TempEntityRefreshCounter::onRefresh>(counter);
        first.triggerFinishOperation();
        temp->requestUpdateEvent().disconnect<TempEntityRefreshCounter,
            &TempEntityRefreshCounter::onRefresh>(counter);
        EXPECT_TRUE(canceledPoint.getAttr("finished").asBool());
        EXPECT_GT(counter.refreshCount, 0) << "point cancellation must remove its preview";
        noListeners();
        EXPECT_EQ(document->entityContainer().asVector().size(), baseline);
        first.cliCommand()->runCommand("PYPOINT");
        first.triggerCoordinateEntered(lc::geo::Coordinate(14, 17));
        EXPECT_EQ(document->entityContainer().asVector().size(), baseline + 1);
        EXPECT_NO_THROW(pybind11::exec(R"py(
points = a.cadMdiChild().document().entityContainer().asVector(32767)
assert all(isinstance(p, lc.entity.Point) for p in points)
assert sum(abs(p.x() - 14) < 1e-9 and abs(p.y() - 17) < 1e-9 for p in points) == 1
)py", ns));
        noListeners();
        undo->undo();
        EXPECT_EQ(document->entityContainer().asVector().size(), baseline);
        EXPECT_FALSE(undo->canUndo());
        undo->redo();
        first.clearUndoableStack();
        first.selectAll();
        ASSERT_EQ(first.cadMdiChild()->selection().size(), baseline + 1);
        first.cliCommand()->runCommand("PYREMOVE");
        EXPECT_TRUE(document->entityContainer().asVector().empty());
        undo->undo();
        EXPECT_EQ(document->entityContainer().asVector().size(), baseline + 1);
        EXPECT_FALSE(undo->canUndo());
        undo->redo();
        EXPECT_TRUE(document->entityContainer().asVector().empty());
        noListeners();
        first.triggerCoordinateEntered(lc::geo::Coordinate(200, 300));
        EXPECT_TRUE(document->entityContainer().asVector().empty());
        EXPECT_EQ(oldDocument->entityContainer().asVector().size(), oldCount);
        EXPECT_EQ(second.currentOperation().getAttr("step").asString(), "AWAIT_PHI");
        EXPECT_FALSE(secondOperation.getAttr("finished").asBool());
        EXPECT_TRUE(second.cadMdiChild()->document()->entityContainer().asVector().empty());
        for (const char* event : {"point", "mouseMove", "number", "text"})
            EXPECT_EQ(second.luaInterface()->listenerCount(event), 1u) << event;
        EXPECT_EQ(lc::ui::python::currentMainWindow(), &second);
    }
    second.triggerFinishOperation();
}

TEST(PyGuiBridgeTest, RetainedCanvasSelectionIsIndependentOfReplacedAndDestroyedViews) {
    ensureUiApplication();
    auto window = std::unique_ptr<lc::ui::MainWindow>(new lc::ui::MainWindow());
    window->cliCommand()->runCommand("PYPOINT");
    window->triggerCoordinateEntered(lc::geo::Coordinate(4, 7));
    auto canvas = window->cadMdiChild()->viewer()->docCanvas();
    auto undo = window->cadMdiChild()->undoManager();
    ASSERT_EQ(canvas->document()->entityContainer().asVector().size(), 1u);
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["window"] = pybind11::cast(window.get(), pybind11::return_value_policy::reference);
    pybind11::exec("canvas = window.cadMdiChild().viewer().docCanvas()", ns);

    // The required offscreen lane proves the no-painter case. A real native
    // backend additionally proves that live point selection uses its painter.
    const auto platform = QGuiApplication::platformName();
    if (platform != "offscreen" && platform != "minimal") {
        window->show();
        QElapsedTimer elapsed;
        elapsed.start();
        while ((!window->cadMdiChild()->viewer()->context() ||
                !window->cadMdiChild()->viewer()->context()->isValid()) && elapsed.elapsed() < 2000)
            QApplication::processEvents(QEventLoop::AllEvents, 20);
        ASSERT_NE(window->cadMdiChild()->viewer()->context(), nullptr);
        ASSERT_TRUE(window->cadMdiChild()->viewer()->context()->isValid());
        EXPECT_NO_THROW(pybind11::exec("canvas.selectPoint(4, 7)", ns));
        EXPECT_EQ(canvas->selectedEntities().asVector().size(), 1u);
        std::cout << "Retained canvas: real " << platform.toStdString()
                  << " painter point selection executed" << std::endl;
    }

    int notifications = 0;
    QObject::connect(window->cadMdiChild(), &lc::ui::CadMdiChild::selectionChangeEvent,
                     window.get(), [&] { ++notifications; });
    window->cadMdiChild()->newDocument();
    notifications = 0;
    EXPECT_NO_THROW(pybind11::exec(R"py(
canvas.removeSelection()
canvas.selectAll()
)py", ns));
    EXPECT_EQ(canvas->selectedEntities().asVector().size(), 1u);
    EXPECT_EQ(notifications, 0) << "retained selection must not notify the replacement view";
    EXPECT_TRUE(window->cadMdiChild()->selection().empty());
    EXPECT_NO_THROW(pybind11::exec(R"py(
canvas.inverseSelection()
try:
    canvas.selectPoint(4, 7)
except RuntimeError as error:
    assert 'painter' in str(error).lower(), str(error)
else:
    raise AssertionError('retained canvas borrowed the replacement painter')
)py", ns));
    EXPECT_EQ(notifications, 0);
    window.reset();
    // A retained document can still remove entities. It must not visit the
    // painter cache that belonged to its destroyed view, even on real Cocoa.
    undo->undo();
    EXPECT_TRUE(canvas->document()->entityContainer().asVector().empty());
    undo->redo();
    EXPECT_EQ(canvas->document()->entityContainer().asVector().size(), 1u);
    EXPECT_NO_THROW(pybind11::exec(R"py(
canvas.selectAll()
canvas.inverseSelection()
canvas.removeSelection()
try:
    canvas.selectPoint(4, 7)
except RuntimeError as error:
    assert 'painter' in str(error).lower(), str(error)
else:
    raise AssertionError('retained canvas used a destroyed view')
)py", ns));
}

TEST(PyGuiBridgeTest, StandaloneCallbacksRouteToTheirWindowAndRestoreScope) {
    ensureUiApplication();
    lc::ui::MainWindow first;
    lc::ui::MainWindow second;
    auto* firstItem = first.addMenu("OwnerTest")->addItem("OwnerItemA");
    auto* secondItem = second.addMenu("OwnerTest")->addItem("OwnerItemB");
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["a"] = pybind11::cast(&first, pybind11::return_value_policy::reference);
    ns["b"] = pybind11::cast(&second, pybind11::return_value_policy::reference);
    ns["item_a"] = pybind11::cast(firstItem, pybind11::return_value_policy::reference);
    ns["item_b"] = pybind11::cast(secondItem, pybind11::return_value_policy::reference);
    pybind11::exec(R"py(
import lc, lcgui as gui
hits = []
def record(label):
    hits.append((label, gui.currentMainWindow()))
    lc.event.register('standalone-owner-test', lambda event, data: None)
assert a.cliCommand().addCommand('OWNER_A', lambda: record('cli-a'))
assert b.cliCommand().addCommand('OWNER_B', lambda: record('cli-b'))
item_a.addCallback(lambda: record('item-a'))
item_b.addCallback(lambda: record('item-b'))
a.connectMenuItem('OwnerItemA', lambda: record('window-a'))
b.connectMenuItem('OwnerItemB', lambda: record('window-b'))
late_menu = gui.Menu('LateMenu')
a.addMenu(late_menu)
nested = gui.Menu('Nested')
late_menu.addMenu(nested)
nested_item = nested.addItem('NestedItem', lambda: record('nested-a'))
)py", ns);
    first.cliCommand()->runCommand("OWNER_A");
    EXPECT_EQ(lc::ui::python::currentMainWindow(), nullptr);
    {
        lc::ui::python::MainWindowExecutionScope scope(&first);
        second.cliCommand()->runCommand("OWNER_B");
        secondItem->trigger();
        EXPECT_EQ(lc::ui::python::currentMainWindow(), &first);
    }
    {
        lc::ui::python::MainWindowExecutionScope scope(&second);
        firstItem->trigger();
        ns["nested_item"].cast<lc::ui::api::MenuItem*>()->trigger();
        EXPECT_EQ(lc::ui::python::currentMainWindow(), &second);
    }
    EXPECT_EQ(first.luaInterface()->listenerCount("standalone-owner-test"), 4u);
    EXPECT_EQ(second.luaInterface()->listenerCount("standalone-owner-test"), 3u);
    EXPECT_NO_THROW(pybind11::exec(R"py(
assert hits == [('cli-a', a), ('cli-b', b), ('item-b', b),
                ('window-b', b), ('item-a', a), ('window-a', a), ('nested-a', a)], hits
)py", ns));
    EXPECT_EQ(lc::ui::python::currentMainWindow(), nullptr);
}

TEST(PyGuiBridgeTest, StandaloneCallbacksHandleLateAdoptionNoOwnerAndDeadOwner) {
    ensureUiApplication();
    auto first = std::make_unique<lc::ui::MainWindow>();
    lc::ui::MainWindow second;
    auto* menu = second.addMenu("AdoptionTest");
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["a"] = pybind11::cast(first.get(), pybind11::return_value_policy::reference);
    ns["b"] = pybind11::cast(&second, pybind11::return_value_policy::reference);
    ns["menu"] = pybind11::cast(menu, pybind11::return_value_policy::reference);
    pybind11::exec(R"py(
import lcgui as gui
hits = []
late = gui.MenuItem.new('Late', lambda: hits.append(('late', gui.currentMainWindow())))
free = gui.MenuItem.new('Free', lambda: hits.append(('free', gui.currentMainWindow())))
menu.addItem(late)
pending = gui.ToolbarButton.new('Pending', '',
    lambda: hits.append(('pending', gui.currentMainWindow())))
group = gui.ToolbarGroup('LateClone')
b.toolbar().tabByName('Quick Access').addGroup(group)
group.addButton(pending)
)py", ns);
    std::unique_ptr<lc::ui::api::ToolbarButton> lateClone(
        ns["pending"].cast<lc::ui::api::ToolbarButton*>()->clone());
    {
        lc::ui::python::MainWindowExecutionScope scope(first.get());
        pybind11::exec(R"py(
saved = gui.ToolbarButton.new('Saved', '',
    lambda: hits.append(('saved', gui.currentMainWindow())))
)py", ns);
        ns["free"].cast<lc::ui::api::MenuItem*>()->trigger();
        ns["late"].cast<lc::ui::api::MenuItem*>()->trigger();
        pybind11::exec("group.removeButton(pending)", ns);
        lateClone->click();
        EXPECT_EQ(lc::ui::python::currentMainWindow(), first.get());
    }
    {
        lc::ui::python::MainWindowExecutionScope scope(&second);
        ns["saved"].cast<lc::ui::api::ToolbarButton*>()->click();
        EXPECT_EQ(lc::ui::python::currentMainWindow(), &second);
        std::unique_ptr<lc::ui::api::ToolbarButton> clone(
            ns["saved"].cast<lc::ui::api::ToolbarButton*>()->clone());
        QPointer<QObject> original = ns["saved"].cast<lc::ui::api::ToolbarButton*>();
        pybind11::exec("del saved", ns);
        ASSERT_TRUE(original.isNull());
        clone->click();
        first.reset();
        clone->click();
        EXPECT_EQ(lc::ui::python::currentMainWindow(), &second);
    }
    EXPECT_NO_THROW(pybind11::exec(R"py(
assert hits == [('free', None), ('late', b), ('pending', b),
                ('saved', a), ('saved', a)], hits
)py", ns));
    EXPECT_EQ(lc::ui::python::currentMainWindow(), nullptr);
}

TEST(PyGuiBridgeTest, EveryGuiCallbackAdapterUsesTheRegisteredWindow) {
    ensureUiApplication();
    lc::ui::MainWindow first;
    lc::ui::MainWindow second;
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["a"] = pybind11::cast(&first, pybind11::return_value_policy::reference);
    ns["b"] = pybind11::cast(&second, pybind11::return_value_policy::reference);
    lc::entity::CADEntity_CSPtr point = std::make_shared<const lc::entity::Point>(
        lc::geo::Coordinate(1, 2, 0), first.cadMdiChild()->activeLayer());
    ns["point"] = pybind11::cast(point);
    ns["dispatch"] = pybind11::cpp_function([](pybind11::object widget, const std::string& slot) {
        const auto lifetime = lc::ui::python::qtLifetime(widget);
        if (!lifetime || !lifetime->object) throw std::runtime_error("Missing actual Qt receiver");
        QObject* receiver = lifetime->object.data();
        bool invoked = true;
        if (slot == "textChangedCallbacks")
            invoked = QMetaObject::invokeMethod(receiver, slot.c_str(), Qt::DirectConnection,
                Q_ARG(QString, QStringLiteral("1")));
        else if (slot == "valueChangedCallbacks")
            invoked = QMetaObject::invokeMethod(receiver, slot.c_str(), Qt::DirectConnection, Q_ARG(double, 2.0));
        else if (slot == "activatedCallbacks")
            invoked = QMetaObject::invokeMethod(receiver, slot.c_str(), Qt::DirectConnection, Q_ARG(int, 0));
        else if (slot == "coordinate-change")
            invoked = QMetaObject::invokeMethod(receiver, "textChangedCallbacks", Qt::DirectConnection);
        else if (slot == "color")
            dynamic_cast<lc::ui::api::ColorGUI*>(receiver)->colorSelectedCallbacks();
        else if (slot == "entity")
            receiver->findChild<QListWidget*>()->setCurrentRow(0);
        else invoked = QMetaObject::invokeMethod(receiver, slot.c_str(), Qt::DirectConnection);
        if (!invoked) throw std::runtime_error("Native GUI slot was not invoked: " + slot);
    });
    {
        lc::ui::python::MainWindowExecutionScope scope(&first);
        pybind11::exec(R"py(
import lc, lcgui as gui
hits, expected, triggers = [], set(), []
def callback(label):
    expected.add(label)
    return lambda *args: hits.append((label, gui.currentMainWindow()))
def watch(widget, method, slot):
    getattr(widget, method)(callback(type(widget).__name__ + '.' + method))
    triggers.append(lambda: dispatch(widget, slot))
menu = gui.Menu('CallbackMatrix')
item = menu.addItem('from-menu', callback('Menu.addItem'))
triggers.append(lambda: dispatch(item, 'trigger'))
factory = gui.MenuItem.new('factory', callback('MenuItem.new'))
factory.addCallback(callback('MenuItem.addCallback'))
factory.addCallback('named', callback('MenuItem.addCallback.named'))
factory.addCheckedCallback(callback('MenuItem.addCheckedCallback'))
triggers.append(lambda: dispatch(factory, 'trigger'))
buttons = [gui.ToolbarButton.new('one', '', callback('ToolbarButton.new')),
           gui.ToolbarButton.newWithTooltip('two', '', callback('ToolbarButton.newWithTooltip'), 'tip'),
           gui.ToolbarButton.newCheckable('three', '', callback('ToolbarButton.newCheckable'), 'tip', True)]
buttons[0].addCallback(callback('ToolbarButton.addCallback'))
buttons[0].addCallback('named', callback('ToolbarButton.addCallback.named'))
triggers.extend(lambda button=button: dispatch(button, 'click') for button in buttons)
group = gui.ToolbarGroup('CallbackMatrix')
group_button = group.addButton('group-button', '', callback('ToolbarGroup.addButton'))
triggers.append(lambda: dispatch(group_button, 'click'))
a.toolbar().addButton('toolbar-button', '', 'CallbackMatrix', callback('Toolbar.addButton'))
toolbar_button = a.toolbar().tabByName('Quick Access').groupByName('CallbackMatrix').buttonByName('toolbar-button')
triggers.append(lambda: dispatch(toolbar_button, 'click'))
dialog = gui.DialogWidget('CallbackMatrix', a)
dialog.addFinishCallback(callback('DialogWidget.addFinishCallback'))
gui.InputGUIContainer.addFinishCallback(dialog, callback('InputGUIContainer.addFinishCallback'))
triggers.append(lambda: dispatch(dialog, 'finishCallbacks'))
text = gui.Text('text')
watch(text, 'addFinishCallback', 'editingFinishedCallbacks')
watch(text, 'addOnChangeCallback', 'textChangedCallbacks')
button = gui.Button('button')
button.addCallback(callback('Button.addCallback'))
triggers.append(button.click)
check = gui.CheckBox('check')
check.addCallback(callback('CheckBox.addCallback'))
triggers.append(lambda: check.setValue(True))
radio = gui.RadioButton('radio')
radio.addCallback(callback('RadioButton.addCallback'))
triggers.append(lambda: radio.setValue(True))
coordinate = gui.Coordinate('coordinate')
watch(coordinate, 'addFinishCallback', 'editingFinishedCallbacks')
coordinate.addOnChangeCallback(callback('Coordinate.addOnChangeCallback'))
triggers.append(lambda: dispatch(coordinate, 'coordinate-change'))
angle = gui.Angle('angle')
watch(angle, 'addFinishCallback', 'editingFinishedCallbacks')
watch(angle, 'addOnChangeCallback', 'textChangedCallbacks')
slider = gui.Slider('slider')
slider.addCallback(callback('Slider.addCallback'))
triggers.append(lambda: slider.setValue(2))
combo = gui.ComboBox('combo')
combo.addItem('one')
combo.addCallback(callback('ComboBox.addCallback'))
triggers.append(lambda: dispatch(combo, 'activatedCallbacks'))
number = gui.Number('number')
watch(number, 'addCallback', 'valueChangedCallbacks')
color = gui.ColorPicker('color')
watch(color, 'addCallback', 'color')
entity = gui.EntityPicker('entity')
dialog.addWidget('entity', entity)
assert gui.InputGUIContainer.keys(dialog) == ['entity']
entity.addEntity(point)
watch(entity, 'addCallback', 'entity')
)py", ns);
    }
    {
        lc::ui::python::MainWindowExecutionScope scope(&second);
        EXPECT_NO_THROW(pybind11::exec("for trigger in triggers: trigger()", ns));
        EXPECT_EQ(lc::ui::python::currentMainWindow(), &second);
    }
    EXPECT_NO_THROW(pybind11::exec(R"py(
assert {label for label, owner in hits} == expected, (expected, hits)
assert all(owner is a for label, owner in hits), hits
assert len(expected) == 28, expected
)py", ns));
    EXPECT_TRUE(first.cadMdiChild()->selection().empty());
}

TEST(PyGuiBridgeTest, ScopedCallbackPreservesEqualityResultsDiagnosticsAndThreadGuard) {
    ensureUiApplication();
    lc::ui::MainWindow first;
    lc::ui::MainWindow second;
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["owner"] = pybind11::cast(&first, pybind11::return_value_policy::reference);
    pybind11::exec(R"py(
import lcgui as gui
class Receiver:
    def callback(self, *args):
        assert gui.currentMainWindow() is owner
        return 42
    def broken(self, *args):
        raise ValueError('scoped callback failure')
receiver = Receiver()
)py", ns);
    auto make = [&](const char* method) {
        return lc::ui::python::scopedUiCallback(lc::python::makePythonCallback(
            ns["receiver"].attr(method)), &first);
    };
    auto callback = make("callback");
    EXPECT_STREQ(callback.runtime(), "python");
    EXPECT_TRUE(callback == make("callback"));
    EXPECT_FALSE(callback == make("broken"));
    std::vector<std::string> diagnostics;
    auto sink = [&](const std::string& message) { diagnostics.push_back(message); };
    {
        lc::ui::python::MainWindowExecutionScope scope(&second);
        EXPECT_EQ(callback.invoke().asInt(), 42);
        EXPECT_EQ(callback.invokeEvent("point", {}, sink).asInt(), 42);
        EXPECT_TRUE(make("broken").invokeEvent("point", {}, sink).isNil());
        ASSERT_EQ(diagnostics.size(), 1u);
        EXPECT_NE(diagnostics[0].find("scoped callback failure"), std::string::npos);
        EXPECT_EQ(lc::ui::python::currentMainWindow(), &second);
    }
    diagnostics.clear();
    std::thread worker([&] { (void)callback.invokeEvent("point", {}, sink); });
    worker.join();
    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_NE(diagnostics[0].find("Qt application thread"), std::string::npos);
    EXPECT_EQ(lc::ui::python::currentMainWindow(), nullptr);
    auto receiver = std::make_unique<QObject>();
    auto orphan = lc::ui::python::scopedUiCallback(
        lc::python::makePythonCallback(ns["receiver"].attr("callback")), receiver.get());
    receiver.reset();
    EXPECT_TRUE(orphan.invoke().isNil());

    lc::ui::api::MenuItem shared("Ambiguous");
    auto late = lc::ui::python::scopedUiCallback(
        lc::python::makePythonCallback(ns["receiver"].attr("callback")), &shared);
    auto* firstMenu = first.addMenu("AmbiguityTest");
    auto* secondMenu = second.addMenu("AmbiguityTest");
    firstMenu->addItem(&shared);
    secondMenu->addItem(&shared);
    EXPECT_THROW(lc::ui::python::scopedUiCallback(
        lc::python::makePythonCallback(ns["receiver"].attr("callback")), &shared), std::runtime_error);
    diagnostics.clear();
    {
        lc::ui::python::MainWindowExecutionScope scope(&second);
        EXPECT_TRUE(late.invokeEvent("point", {}, sink).isNil());
        ASSERT_EQ(diagnostics.size(), 1u);
        EXPECT_NE(diagnostics[0].find("ambiguous associated windows"), std::string::npos);
        secondMenu->removeAction(&shared);
        EXPECT_EQ(late.invoke().asInt(), 42);
        // Once adopted, subsequent movement cannot steal the registered owner.
        firstMenu->removeAction(&shared);
        secondMenu->addAction(&shared);
        EXPECT_EQ(late.invoke().asInt(), 42);
        EXPECT_EQ(lc::ui::python::currentMainWindow(), &second);
    }
}

TEST(PyGuiBridgeTest, RetainedQtHandlesRejectDestroyedOwner) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    auto window = std::make_unique<lc::ui::MainWindow>();
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["window"] = pybind11::cast(window.get(), pybind11::return_value_policy::reference);
    // Fail safely on an unguarded implementation before invoking a dangling
    // receiver. Then exercise actual bound methods after native destruction.
    ASSERT_TRUE(pybind11::hasattr(ns["window"], "_lc_qobject_lifetime"));
    pybind11::exec(R"py(
mdi = window.cadMdiChild()
cli = window.cliCommand()
lua = window.luaInterface()
assert isinstance(mdi.getFilename(), str)
assert isinstance(cli.availableCommands(), list)
calls = [lambda: window.message("stale"), mdi.getFilename,
         cli.availableCommands, lambda: lua.pluginList("")]
)py", ns);
    window.reset();
    EXPECT_NO_THROW(pybind11::exec(R"py(
for call in calls:
    try:
        call()
    except RuntimeError as error:
        assert "destroyed Qt object" in str(error), str(error)
    else:
        raise AssertionError("destroyed receiver was accepted")
)py", ns));
}

TEST(PyGuiBridgeTest, DestroyedQtArgumentsAndOwnedChildrenAreSafe) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    auto owner = std::make_unique<lc::ui::MainWindow>();
    lc::ui::MainWindow other;
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["owner"] = pybind11::cast(owner.get(), pybind11::return_value_policy::reference);
    ns["other"] = pybind11::cast(&other, pybind11::return_value_policy::reference);
    ASSERT_TRUE(pybind11::hasattr(ns["owner"], "_lc_qobject_lifetime"));
    pybind11::exec(R"py(
import lcgui
import gc
import weakref
menu = owner.addMenu("lifetime probe")
submenu = menu.addMenu("child menu")
item = submenu.addItem("child item")
dialog = lcgui.DialogWidget("lifetime probe", owner)
assert isinstance(dialog.keys(), list)  # mixed non-QObject base API
field = lcgui.Text("field")
assert dialog.addWidget("field", field)
field.setValue("live")
assert field.value() == "live"
cycle = lcgui.Text("callback-held child")
assert dialog.addWidget("cycle", cycle)
cycle.addFinishCallback(lambda widget=cycle: None)
cycle_ref = weakref.ref(cycle)
del cycle
gc.collect()
assert cycle_ref() is not None  # The C++ callback, not this dict, roots it.
)py", ns);
    owner.reset();
    EXPECT_NO_THROW(pybind11::exec(R"py(
for name, call in (("menu argument", lambda: other.addMenu(menu)),
                   ("submenu", submenu.label), ("item", item.label),
                   ("dialog base", dialog.keys), ("dialog", dialog.enable),
                   ("field", field.value),
                   ("constructor owner", lambda: lcgui.DialogWidget("invalid owner", owner))):
    try:
        call()
    except RuntimeError as error:
        assert "destroyed Qt object" in str(error), str(error)
    else:
        raise AssertionError("destroyed Qt argument/child was accepted: " + name)
# Qt already destroyed this Python-created child. Collecting the retained
# wrapper must not delete it a second time.
assert cycle_ref() is None  # Qt teardown released the callback-held wrapper.
del dialog, field, menu, submenu, item, owner, call
gc.collect()
)py", ns));
}

TEST(PyGuiBridgeTest, NativeQtAddressReuseDoesNotReviveStaleWrapper) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    using Text = lc::ui::api::TextGUI;
    alignas(Text) unsigned char storage[sizeof(Text)];
    auto destroy = [](Text* widget) { widget->~Text(); };
    std::unique_ptr<Text, decltype(destroy)> widget(new (storage) Text("first"), destroy);
    pybind11::gil_scoped_acquire gil;
    auto first = pybind11::cast(widget.get(), pybind11::return_value_policy::reference);
    ASSERT_TRUE(pybind11::hasattr(first, "_lc_qobject_lifetime"));
    widget.reset();
    widget.reset(new (storage) Text("replacement"));
    auto second = pybind11::cast(widget.get(), pybind11::return_value_policy::reference);
    EXPECT_FALSE(first.is(second));
    EXPECT_NO_THROW(second.attr("setValue")("replacement"));
    EXPECT_EQ(second.attr("value")().cast<std::string>(), "replacement");
    EXPECT_THROW(first.attr("value")(), pybind11::error_already_set);
    widget.reset();
}

TEST(PyGuiBridgeTest, QtAdoptionSurvivesWrapperCollectionWithoutLeakingStandaloneWidgets) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    auto owner = std::make_unique<lc::ui::MainWindow>();
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["owner"] = pybind11::cast(owner.get(), pybind11::return_value_policy::reference);
    pybind11::exec(R"py(
import lcgui, gc, weakref
dialog = lcgui.DialogWidget("adoption probe", owner)
field = lcgui.Text("adopted")
standalone = lcgui.Text("standalone")
item_factory = lcgui.MenuItem.new("factory item", lambda: None)
button_factory = lcgui.ToolbarButton.new("factory button", "", lambda: None)
tooltip_factory = lcgui.ToolbarButton.newWithTooltip("factory tooltip", "", lambda: None, "tip")
checkable_factory = lcgui.ToolbarButton.newCheckable("factory checkable", "", lambda: None, "tip", True)
assert dialog.addWidget("field", field)
field.setValue("retained by Qt")
field_ref = weakref.ref(field)
)py", ns);
    QPointer<lc::ui::api::TextGUI> field(ns["field"].cast<lc::ui::api::TextGUI*>());
    QPointer<lc::ui::api::TextGUI> standalone(ns["standalone"].cast<lc::ui::api::TextGUI*>());
    const std::vector<QPointer<QObject>> factories {
        ns["item_factory"].cast<lc::ui::api::MenuItem*>(),
        ns["button_factory"].cast<lc::ui::api::ToolbarButton*>(),
        ns["tooltip_factory"].cast<lc::ui::api::ToolbarButton*>(),
        ns["checkable_factory"].cast<lc::ui::api::ToolbarButton*>()
    };
    pybind11::exec(R"py(
del field, standalone, item_factory, button_factory, tooltip_factory, checkable_factory
gc.collect()
assert field_ref() is None
)py", ns);
    ASSERT_FALSE(field.isNull());  // Fail before ever dereferencing a deleted child.
    EXPECT_TRUE(standalone.isNull());
    for (const auto& factory : factories) EXPECT_TRUE(factory.isNull());
    EXPECT_EQ(field->value(), "retained by Qt");
    owner.reset();
    EXPECT_TRUE(field.isNull());
}

// -----------------------------------------------------------------------------
// Test — MainWindow method surface: exercise the phase-5 PR-5.6 additions
// (toolbar/layers/menuByName/currentOperation/...) via a real MainWindow.
//
// This is what code review MISSED for PR-5.1 fixups: whether the .h
// signature actually matches the .def(...) shape.  The test asks Python
// to call each new method — if any binding is misnamed or referenced by
// a signature that doesn't exist, pybind11 raises at call time.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, MainWindowRemainderBindingsCallable) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mw;
    lc::ui::python::MainWindowExecutionScope ownerScope(&mw);

    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");
    ns["mainWindow"]   = pybind11::cast(
        &mw, pybind11::return_value_policy::reference);

    // Reach through each new MainWindow method the fixup added.  These
    // are all reference-returning accessors — if a Getter is wired to a
    // signature that doesn't exist, this line raises with
    // "AttributeError" or "TypeError" at py::exec.
    pybind11::exec(R"py(
import lcgui

assert lcgui.currentMainWindow() is not None, \
    "currentMainWindow() should find the registered MainWindow"

# Reference-returning accessors — cover Toolbar, Layers, LuaInterface,
# CliCommand, CadMdiChild.
toolbar   = mainWindow.toolbar()
layers    = mainWindow.layers()
lua_iface = mainWindow.luaInterface()
cli       = mainWindow.cliCommand()
mdi       = mainWindow.cadMdiChild()

assert toolbar   is not None
assert layers    is not None
assert lua_iface is not None
assert cli       is not None
assert mdi       is not None

# CadMdiChild remainder methods (metaInfoManager / getSnapManager /
# getFilename / activeViewport).  Any wrong .h signature raises here.
mim  = mdi.metaInfoManager()
snap = mdi.getSnapManager()
name = mdi.getFilename()
vp   = mdi.activeViewport()

assert mim  is not None
assert snap is not None
assert isinstance(name, str)
# activeViewport CAN be a None-holder shared_ptr but the method must
# not raise.

# LuaInterface remainder — pluginList takes a path arg and returns a
# list of strings.
lst = lua_iface.pluginList("../does-not-exist")
assert isinstance(lst, list)

# PR-5.7 fixup — currentOperation() / operation() / setOperation()
# bindings REMOVED because they return/take lc::scripting::ScriptObject
# which isn't registered with pybind11.  The test used to exercise
# `mainWindow.currentOperation()` here — which would have raised
# `TypeError: Unregistered type` if the test binary had ever been
# run.  See the pyguibridge.cpp inline note.
assert not hasattr(mainWindow, "currentOperation"), \
    "currentOperation must not be bound — see ScriptObject note in pyguibridge.cpp"
assert not hasattr(lua_iface, "operation"), \
    "LuaInterface.operation must not be bound — see ScriptObject note"
assert not hasattr(lua_iface, "setOperation"), \
    "LuaInterface.setOperation must not be bound — see ScriptObject note"

# menuByPosition on an empty menu bar — returns None or a menu; either
# is fine as long as the call doesn't raise.
_ = mainWindow.menuByPosition(0)
)py",
        ns);

    // The Lua GUI bridge exposes the plain constructor and callback factory
    // under the same `new` name. Exercise both overloads to prevent the
    // Kaguya duplicate-registration regression from returning.
    const int luaStatus = luaL_dostring(mw.luaInterface()->luaState(), R"lua(
local plainItem = gui.MenuItem.new("Plain")
assert(plainItem:label() == "Plain")
local callbackItem = gui.MenuItem.new("Callback", function() end)
assert(callbackItem:label() == "Callback")

local plainButton = gui.ToolbarButton.new("PlainButton", ":/icons/none.svg", "tip", true)
assert(plainButton:label() == "PlainButton")
local callbackButton = gui.ToolbarButton.new("CallbackButton", ":/icons/none.svg", function() end)
assert(callbackButton:label() == "CallbackButton")
)lua");
    if (luaStatus != 0) {
        const char* error = lua_tostring(mw.luaInterface()->luaState(), -1);
        FAIL() << "Lua GUI constructor overloads failed: "
               << (error == nullptr ? "unknown Lua error" : error);
    }
}

// -----------------------------------------------------------------------------
// Test — SnapManager setters actually mutate state.  Bound as a
// shared_ptr class; Python reaches it via CadMdiChild.getSnapManager().
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, SnapManagerSettersMutate) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mw;
    MainWindowRegistration mwReg(&mw);

    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");
    ns["mainWindow"]   = pybind11::cast(
        &mw, pybind11::return_value_policy::reference);

    // Toggle each snap-manager setter — if any of the four bound
    // functions references a wrong .h signature (e.g. name typo like
    // `setIntersectionSnappable` vs `setIntersectionsSnappable`),
    // pybind11 raises at py::exec.  This is exactly the discipline the
    // coordinator called out.
    pybind11::exec(R"py(
snap = mainWindow.cadMdiChild().getSnapManager()
assert snap is not None
snap.setGridSnappable(True)
snap.setGridSnappable(False)
snap.setIntersectionSnappable(True)
snap.setMiddleSnappable(False)
snap.setEntitySnappable(True)
)py",
        ns);
}

// -----------------------------------------------------------------------------
// Test — CliCommand full surface: verifies addCommand-with-Python
// callback goes through the ScriptCallback pipeline, plus the enable /
// availableCommands / commandsHistory / clear surface.  The .c_str()
// bug that shipped in PR-5.5 (fixup round 1) would fail here with a
// TypeError if it recurred.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, CliCommandFullSurfaceCallable) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mw;
    MainWindowRegistration mwReg(&mw);

    pybind11::gil_scoped_acquire gil;
    // Use builtins as a persistent scratchpad so test state survives
    // exec() boundaries — same pattern as testtoolbarbutton.cpp.
    pybind11::module_::import("builtins").attr("_lc_cli_hits") =
        pybind11::list();

    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");
    ns["mainWindow"]   = pybind11::cast(
        &mw, pybind11::return_value_policy::reference);

    pybind11::exec(R"py(
import builtins
cli = mainWindow.cliCommand()

# addCommand — Python callable wrapped as ScriptCallback.  Registering
# a fresh name must return True; duplicate must return False.
ok = cli.addCommand("PYUNIT",
    lambda: builtins._lc_cli_hits.append("PYUNIT_FIRED"))
assert ok is True, "first addCommand for a new name must succeed"

dup = cli.addCommand("PYUNIT", lambda: None)
assert dup is False, "duplicate addCommand must return False"

# Read the full-surface API — every one MUST succeed regardless of
# whether commands run.
assert isinstance(cli.availableCommands(), list)
assert isinstance(cli.commandsHistory(),  list)

# enable/disable + isCommandEnabled round-trip.
cli.disableCommand("PYUNIT")
assert cli.isCommandEnabled("PYUNIT") is False
cli.enableCommand("PYUNIT")
assert cli.isCommandEnabled("PYUNIT") is True

# clear() must be callable.  write() must be callable.
cli.write("smoke test message")
# don't call clear() — a smoke line above is useful debug on failure
)py",
        ns);

    pybind11::module_::import("builtins").attr("_lc_cli_hits") =
        pybind11::list();
}

// -----------------------------------------------------------------------------
// Test — PropertyEditor::GetPropertyEditor accepts a MainWindow arg and
// returns a pointer.  Static bound method; if def_static routing is
// wrong the call raises TypeError.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, PropertyEditorGetPropertyEditor) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mw;
    MainWindowRegistration mwReg(&mw);

    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");
    ns["mainWindow"]   = pybind11::cast(
        &mw, pybind11::return_value_policy::reference);

    pybind11::exec(R"py(
import lcgui
pe = lcgui.PropertyEditor.GetPropertyEditor(mainWindow)
assert pe is not None, "GetPropertyEditor must return the singleton"

# propertyChanged is bound as a normal method.  Fires against an
# unknown key — should be a no-op, not raise.
pe.propertyChanged("some.unknown.key")
)py",
        ns);
}

// -----------------------------------------------------------------------------
// Test — Layers.addLayer overload dispatch works.  Layers has 7 addLayer
// overloads bound via 7 explicit lambdas; pybind11 must resolve by
// argument arity + type.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, LayersOverloadDispatch) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mw;
    MainWindowRegistration mwReg(&mw);

    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");
    ns["mainWindow"]   = pybind11::cast(
        &mw, pybind11::return_value_policy::reference);

    pybind11::exec(R"py(
import lc

layers = mainWindow.layers()

# 1-arg addLayer(name) — returns a Layer_CSPtr.
l1 = layers.addLayer("pygui_test_A")
assert l1 is not None

# 2-arg addLayer(name, lineWidth).
l2 = layers.addLayer("pygui_test_B", 2.5)
assert l2 is not None

# 4-arg addLayer(name, r, g, b).
l3 = layers.addLayer("pygui_test_C", 128, 64, 32)
assert l3 is not None

# 2-arg addLayer(name, color) — same arg count as (name, lineWidth) but
# second arg is a Color object; pybind11 resolves by type.
color = lc.Color(0, 255, 0)
l4 = layers.addLayer("pygui_test_D", color)
assert l4 is not None

# 3-arg addLayer(name, lineWidth, color).
l5 = layers.addLayer("pygui_test_E", 1.0, color)
assert l5 is not None

# 5-arg addLayer(name, lineWidth, r, g, b).
l6 = layers.addLayer("pygui_test_F", 0.5, 200, 100, 50)
assert l6 is not None

# layerByName round-trip.
found = layers.layerByName("pygui_test_A")
assert found is not None

# removeLayer(name) and removeLayer(Layer_CSPtr).
layers.removeLayer("pygui_test_A")
layers.removeLayer(l2)

# renameLayer(name, newName).
layers.renameLayer("pygui_test_C", "pygui_test_C_renamed")

# layers() enumeration.
all_layers = layers.layers()
assert isinstance(all_layers, list)
)py",
        ns);
}

// -----------------------------------------------------------------------------
// Test — gui.* Menu widget family construction + method surface.
// PR-5.6 sub-piece 2 additions.  Constructs Menu/MenuItem/ToolbarTab/
// ToolbarButton/ToolbarGroup instances, calls representative methods
// on each, verifies overloaded ctor arities resolve.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, MenuFamilyConstructAndCall) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    // No MainWindow needed — these widgets are constructible standalone.
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");

    pybind11::exec(R"py(
import lcgui

# Menu: (name) ctor + label round-trip + position + setPosition.
menu = lcgui.Menu("PyTest File")
assert menu.label() == "PyTest File"
menu.setLabel("Renamed")
assert menu.label() == "Renamed"
menu.setPosition(0)
assert menu.position() == 0

# addItem(label) → MenuItem*.  Verifies the label-only overload.
item = menu.addItem("PyMenuItem")
assert item is not None
assert item.label() == "PyMenuItem"

# addItem(label, cb) — callback-taking overload.
hit_counter = {"n": 0}
def cb():
    hit_counter["n"] += 1

item_with_cb = menu.addItem("WithCB", cb)
assert item_with_cb is not None

# itemByName round-trip.
found = menu.itemByName("PyMenuItem")
assert found is not None
assert found.label() == "PyMenuItem"

# QMenu-inherited hide/show/setEnabled/isEnabled.
menu.show()
menu.hide()
menu.setEnabled(False)
assert menu.isEnabled() is False
menu.setEnabled(True)

# addMenu(label) — nested menu.
submenu = menu.addMenu("SubMenu")
assert submenu is not None
assert submenu.label() == "SubMenu"

# removeItem/removeMenu.
menu.removeItem("WithCB")
menu.removeMenu("SubMenu")
)py",
        ns);
}

// -----------------------------------------------------------------------------
// Test — MenuItem plain ctor + `.new(label, cb)` static factory + full
// method surface (addCallback overloads, addCheckedCallback,
// setCheckable/setChecked, hide/show/setEnabled, remove).
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, MenuItemConstructAndCall) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    pybind11::gil_scoped_acquire gil;
    pybind11::module_::import("builtins").attr("_lc_menu_hits") =
        pybind11::list();

    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");

    pybind11::exec(R"py(
import lcgui, builtins

# Plain (label) ctor.
mi = lcgui.MenuItem("PyItem")
assert mi.label() == "PyItem"

# .new(label, cb) static factory — the callback-taking form.
mi_cb = lcgui.MenuItem.new("PyItemCB",
    lambda: builtins._lc_menu_hits.append("PyItemCB fired"))
assert mi_cb is not None
assert mi_cb.label() == "PyItemCB"

# addCallback(callback) and addCallback(name, callback) overloads.
mi.addCallback(lambda: builtins._lc_menu_hits.append("mi anon"))
mi.addCallback("named", lambda: builtins._lc_menu_hits.append("mi named"))

# addCheckedCallback.
mi.addCheckedCallback(
    lambda checked: builtins._lc_menu_hits.append(("checked", checked)))

# setCheckable / setChecked — QAction-inherited, must not raise.
mi.setCheckable(True)
mi.setChecked(True)

# hide/show/setEnabled/isEnabled.
mi.hide()
mi.show()
mi.setEnabled(False)
assert mi.isEnabled() is False
mi.setEnabled(True)

# Position round-trip.
mi.setPosition(2)
assert mi.position() == 2

# removeCallback by name (the named callback registered above).
mi.removeCallback("named")

# setLabel round-trip.
mi.setLabel("PyItem_renamed")
assert mi.label() == "PyItem_renamed"
)py",
        ns);

    pybind11::module_::import("builtins").attr("_lc_menu_hits") =
        pybind11::list();
}

// -----------------------------------------------------------------------------
// Test — Toolbar widget family construction (ToolbarTab / ToolbarButton /
// ToolbarGroup).  Verifies the 3 ToolbarButton ctor arities, static
// factories, and inter-widget composition (button in group in tab).
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, ToolbarWidgetFamilyConstructAndCompose) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    pybind11::gil_scoped_acquire gil;
    pybind11::module_::import("builtins").attr("_lc_tbb_hits") =
        pybind11::list();

    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");

    pybind11::exec(R"py(
import lcgui, builtins

# ToolbarTab: single (name) ctor.
tab = lcgui.ToolbarTab("PyTab")
assert tab.label() == "PyTab"
tab.setLabel("PyTab_2")
assert tab.label() == "PyTab_2"

# ToolbarGroup: 2 ctor arities — (name) and (name, width).
group = lcgui.ToolbarGroup("PyGroup")
assert group.label() == "PyGroup"

group_w = lcgui.ToolbarGroup("PyGroupW", 5)
assert group_w.width() == 5

# ToolbarButton: 3 plain ctor arities.
btn_a = lcgui.ToolbarButton("BtnA", ":/icons/none.svg")
assert btn_a.label() == "BtnA"

btn_b = lcgui.ToolbarButton("BtnB", ":/icons/none.svg", "tooltip B")
assert btn_b.label() == "BtnB"

btn_c = lcgui.ToolbarButton("BtnC", ":/icons/none.svg", "tooltip C", True)
assert btn_c.label() == "BtnC"

# ToolbarButton static factories — callback-taking.
btn_new = lcgui.ToolbarButton.new("BtnNew", ":/icons/none.svg",
    lambda: builtins._lc_tbb_hits.append("BtnNew fired"))
assert btn_new.label() == "BtnNew"

btn_tt = lcgui.ToolbarButton.newWithTooltip("BtnTT", ":/icons/none.svg",
    lambda: builtins._lc_tbb_hits.append("BtnTT fired"), "the tooltip")
assert btn_tt.label() == "BtnTT"

btn_chk = lcgui.ToolbarButton.newCheckable("BtnChk", ":/icons/none.svg",
    lambda: builtins._lc_tbb_hits.append("BtnChk fired"),
    "check tip", True)
assert btn_chk.label() == "BtnChk"

# Composition: add group to tab, add button to group, verify lookup.
tab.addGroup(group)
group.addButton(btn_a)
group.addButton("BtnByStr", ":/icons/none.svg")
group.addButton("BtnByCB", ":/icons/none.svg",
    lambda: builtins._lc_tbb_hits.append("BtnByCB fired"))

found = group.buttonByName("BtnA")
assert found is not None

# ToolbarGroup.buttons() returns list.
btns = group.buttons()
assert isinstance(btns, list)

# ToolbarTab.groups() returns list.
groups = tab.groups()
assert isinstance(groups, list)

# QWidget-inherited: hide/show/enable/disable + isEnabled.
tab.enable(); tab.disable(); tab.enable()
group.hide(); group.show()
group.enable(); group.disable(); group.enable()
btn_a.hide(); btn_a.show()
btn_a.enable(); btn_a.disable(); btn_a.enable()
assert btn_a.isEnabled() is True
)py",
        ns);

    pybind11::module_::import("builtins").attr("_lc_tbb_hits") =
        pybind11::list();
}

// -----------------------------------------------------------------------------
// Test — Input widget family construction + value round-trips.
// PR-5.6 sub-piece 3 additions.  Constructs every input-widget subclass
// bound in lcgui and exercises its value/setValue/addCallback surface.
// The critical failure mode this test guards: pybind11's inheritance
// chain — every widget subclass declares InputGUI as base.  If the
// base binding is missing, addWidget-into-InputGUIContainer paths fail
// with "type not registered" at py::exec.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, InputWidgetsConstructAndRoundTrip) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    pybind11::gil_scoped_acquire gil;
    pybind11::module_::import("builtins").attr("_lc_iw_hits") =
        pybind11::list();

    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");

    pybind11::exec(R"py(
import lcgui, lc, builtins

# TextGUI (Text).  value/setValue round-trip + callback.
t = lcgui.Text("PyText")
assert t.label() == "PyText"
t.setValue("hello")
assert t.value() == "hello"
t.addFinishCallback(lambda: builtins._lc_iw_hits.append("Text.finish"))
t.addOnChangeCallback(lambda: builtins._lc_iw_hits.append("Text.change"))

# ButtonGUI (Button).  No value.  Just callback + click.
b = lcgui.Button("PyBtn")
b.addCallback(lambda: builtins._lc_iw_hits.append("Button.fire"))
b.click()
assert "Button.fire" in builtins._lc_iw_hits

# CheckBoxGUI (CheckBox).  Two ctor arities (label, label+checked).
c1 = lcgui.CheckBox("PyChk1")
assert c1.value() is False
c1.setValue(True)
assert c1.value() is True

c2 = lcgui.CheckBox("PyChk2", True)
assert c2.value() is True

# RadioButtonGUI (RadioButton).  Standalone (does not inherit InputGUI).
r = lcgui.RadioButton("PyRadio")
assert r.label() == "PyRadio"
r.setValue(True)
assert r.value() is True

# HorizontalGroupGUI (HorizontalGroup).  addWidget takes InputGUI*.
hg = lcgui.HorizontalGroup("PyHGroup")
hg.addWidget("t1", t)
hg.addWidget("b1", b)  # ButtonGUI upcast to InputGUI* via pybind11

# Vertical variant via 2-arg ctor.
vg = lcgui.HorizontalGroup("PyVGroup", True)
assert vg.label() == "PyVGroup"

# RadioGroupGUI (RadioGroup).  addButton takes (key, RadioButtonGUI*).
rg = lcgui.RadioGroup("PyRadioGroup")
rg.addButton("r1", r)

# CoordinateGUI (Coordinate).  value returns lc::geo::Coordinate.
coord = lcgui.Coordinate("PyCoord")
coord.setValue(lc.geo.Coordinate(1.0, 2.0, 3.0))
v = coord.value()
assert abs(v.x() - 1.0) < 1e-9
assert abs(v.y() - 2.0) < 1e-9

# AngleGUI (Angle).  value returns double.
ang = lcgui.Angle("PyAngle")
ang.setValue(1.57)
assert abs(ang.value() - 1.57) < 1e-9
ang.toDegrees()
ang.toRadians()

# SliderGUI (Slider).  2 ctor arities.
s1 = lcgui.Slider("PySlider1")
s2 = lcgui.Slider("PySlider2", 0, 200)
s2.setValue(100)
assert s2.value() == 100

# ComboBoxGUI (ComboBox).  addItem + setValue (2 overloads).
cb = lcgui.ComboBox("PyCombo")
cb.addItem("one")
cb.addItem("two", 0)  # insert at index 0
cb.setValue("one")
assert cb.value() == "one"
cb.setValue(0)  # by index

# NumberGUI (Number).  2 ctor arities.
n1 = lcgui.Number("PyNum")
n2 = lcgui.Number("PyNumRange", -100.0, 100.0)
n2.setValue(42.5)
assert abs(n2.value() - 42.5) < 1e-9

# ColorGUI (ColorPicker).  value returns lc.Color.
cp = lcgui.ColorPicker("PyColor")
cp.setValue(lc.Color(200, 100, 50))
col = cp.value()
# lc.Color is bound in lc kernel — accessor names vary but the object
# must round-trip.
assert col is not None

# EntityGUI (EntityPicker).  value returns list of CADEntity_CSPtr;
# empty by default.
ep = lcgui.EntityPicker("PyEnt")
lst = ep.value()
assert isinstance(lst, list)

# ListGUI (List).  addItem takes an InputGUI child; setListType by
# string.  DELIBERATELY NO addCallbackToAll — Lua's surface excludes
# it and Python mirrors that.
li = lcgui.List("PyList")
li.setListType("NONE")
assert not hasattr(li, "addCallbackToAll"), \
    "List must NOT expose addCallbackToAll (Lua parity)"
)py",
        ns);

    pybind11::module_::import("builtins").attr("_lc_iw_hits") =
        pybind11::list();
}

// -----------------------------------------------------------------------------
// Test — DialogWidget + InputGUIContainer composition.  Constructs a
// DialogWidget, adds child widgets by key, verifies keys() enumerates
// them, and calls addFinishCallback.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, BoundPolymorphicBasesHaveVirtualDestructors) {
    static_assert(
        std::has_virtual_destructor<lc::ui::api::InputGUIContainer>::value,
        "InputGUIContainer is a polymorphic Python-visible base");
    static_assert(
        std::has_virtual_destructor<lc::viewer::drawable::TempEntities>::value,
        "TempEntities is a polymorphic Python-visible type");
    SUCCEED();
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, DialogWidgetComposition) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mw;
    MainWindowRegistration mwReg(&mw);

    pybind11::gil_scoped_acquire gil;
    pybind11::module_::import("builtins").attr("_lc_dlg_hits") =
        pybind11::list();

    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");
    ns["mainWindow"]   = pybind11::cast(
        &mw, pybind11::return_value_policy::reference);

    pybind11::exec(R"py(
import lcgui, builtins

# DialogWidget ctor takes (title, MainWindow*).
dlg = lcgui.DialogWidget("PyDialog", mainWindow)
assert dlg.keys() == [], "new DialogWidget should start without registered keys"

# Compose child widgets via addWidget under keys.
text_widget = lcgui.Text("field_A")
num_widget  = lcgui.Number("field_B")
ok_button   = lcgui.Button("OK")

assert dlg.addWidget("A", text_widget) is True
assert dlg.keys() == ["A"], "first key must be registered"
assert dlg.addWidget("B", num_widget)  is True
assert dlg.addWidget("btn", ok_button) is True

# Duplicate key must be rejected.
assert dlg.addWidget("A", lcgui.Text("dupe")) is False

# keys() enumerates.
keys = dlg.keys()
assert isinstance(keys, list)
assert set(keys) >= {"A", "B", "btn"}

# inputWidgets() enumerates widget objects.
widgets = dlg.inputWidgets()
assert isinstance(widgets, list)
assert len(widgets) >= 3

# setFinishButton + addFinishCallback wire up the finish path.
dlg.setFinishButton(ok_button)
dlg.addFinishCallback(
    lambda args: builtins._lc_dlg_hits.append("finish"))

# enable/disable round-trip.
dlg.enable()
dlg.disable()
dlg.enable()
)py",
        ns);

    pybind11::module_::import("builtins").attr("_lc_dlg_hits") =
        pybind11::list();
}

// Exercise the real rectangle custom-entity callbacks through a mutable
// transaction builder. The release re-executes the same Builder that was
// committed at press, so one undo must restore the original rectangle and one
// redo must restore the resized one without duplicate entities.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, PythonRectangleDragIsOneUndoStep) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    TempEntityRefreshCounter previewRefresh;
    lc::ui::MainWindow mw;
    MainWindowRegistration mwReg(&mw);
    lc::ui::python::MainWindowExecutionScope ownerScope(&mw);
    auto tempEntities = mw.cadMdiChild()->tempEntities();
    tempEntities->requestUpdateEvent().connect<
        TempEntityRefreshCounter,
        &TempEntityRefreshCounter::onRefresh>(previewRefresh);

    pybind11::gil_scoped_acquire gil;
    pybind11::module_::import("sys").attr("path").attr("insert")(
        0, QCoreApplication::applicationDirPath().toStdString());

    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");
    ns["mainWindow"] = pybind11::cast(
        &mw, pybind11::return_value_policy::reference);

    pybind11::exec(R"py(
import importlib
import lc

rectangle = importlib.import_module('lcUIPy.plugins.rectangle.plugin')
mdi = mainWindow.cadMdiChild()
doc = mdi.document()
layer = mdi.activeLayer()
undo = mdi.undoManager()

base = lc.geo.Coordinate(1.0, 2.0, 0.0)
old_block = rectangle._create_storage(base, 10.0, 8.0)
lc.operation.AddBlock.new(doc, old_block).execute()
old_insert = rectangle._create_rectangle_entity(
    base, layer, doc, old_block)
try:
    rectangle._create_rectangle_entity(
        base, layer, doc, old_block, object())
except TypeError:
    pass
else:
    raise AssertionError('invalid MetaInfo was silently discarded')
initial_entities = lc.operation.EntityBuilder.new(doc)
for line in rectangle._generate_lines(
        base, lc.geo.Coordinate(11.0, 10.0, 0.0), layer, old_block):
    initial_entities.appendEntity(line)
initial_entities.appendEntity(old_insert)
initial_entities.execute()
setup_entities = doc.entityContainer().asVector(32767)
assert len(setup_entities) == 1, f'rectangle setup created {len(setup_entities)} top-level entities'
assert len(doc.entitiesByBlock(old_block).asVector(32767)) == 4

# Treat setup as the pre-existing document state; only the drag transaction
# should remain on the undo stack.
undo.removeUndoables()
drag = lc.operation.Builder(doc, 'Python rectangle drag')
rectangle._drag_point_click(old_insert, drag, 2)
drag.execute()
assert len(doc.entityContainer().asVector(32767)) == 0, 'press callback did not remove the original insert'
assert len(doc.entitiesByBlock(old_block).asVector(32767)) == 0, 'press callback did not remove the original block geometry'

# Move the top-right corner from (11, 10) to (13, 11), producing a 12x9
# rectangle while keeping its lower-left corner fixed at (1, 2).
rectangle._new_drag_point(
    old_insert, lc.geo.Coordinate(13.0, 11.0, 0.0))
rectangle._drag_point_release(old_insert, drag)
drag.execute()

def custom_rectangles():
    return [entity for entity in doc.entityContainer().asVector(32767)
            if isinstance(entity, lc.entity.ScriptCustomEntity)]

assert len(doc.entityContainer().asVector(32767)) == 1, 'release did not leave exactly one rectangle insert'
resized = custom_rectangles()
assert len(resized) == 1, 'release did not leave exactly one custom rectangle'
new_block = resized[0].displayBlock()
assert len(doc.entitiesByBlock(new_block).asVector(32767)) == 4, 'release did not create exactly four block edges'
assert float(new_block.param('width')) == 12.0, 'drag width was not applied'
assert float(new_block.param('height')) == 9.0, 'drag height was not applied'
assert undo.canUndo() is True, 'drag did not create an undo entry'

# One undo restores the original 4 lines + custom entity. A second undo
# would consume unrelated document history and must not be needed.
undo.undo()
assert undo.canUndo() is False, 'one undo did not consume the full drag transaction'
assert undo.canRedo() is True, 'undo did not enable redo'
restored = custom_rectangles()
assert len(doc.entityContainer().asVector(32767)) == 1, 'undo did not restore exactly one original insert'
assert len(restored) == 1, 'undo did not restore exactly one custom rectangle'
assert len(doc.entitiesByBlock(old_block).asVector(32767)) == 4, 'undo did not restore the original block geometry'
assert restored[0].id() == old_insert.id(), 'undo did not restore the original entity ID'
assert float(restored[0].displayBlock().param('width')) == 10.0, 'undo did not restore the original width'
assert float(restored[0].displayBlock().param('height')) == 8.0, 'undo did not restore the original height'

undo.redo()
assert undo.canRedo() is False, 'redo did not consume the drag redo entry'
assert len(doc.entityContainer().asVector(32767)) == 1, 'redo created duplicate or missing rectangle inserts'
resized = custom_rectangles()
assert len(resized) == 1, 'redo did not restore exactly one custom rectangle'
assert len(doc.entitiesByBlock(resized[0].displayBlock()).asVector(32767)) == 4, 'redo did not restore exactly four block edges'
assert float(resized[0].displayBlock().param('width')) == 12.0, 'redo did not restore the resized width'
assert float(resized[0].displayBlock().param('height')) == 9.0, 'redo did not restore the resized height'
)py",
        ns);

    EXPECT_EQ(previewRefresh.refreshCount, 16)
        << "Python rectangle drag must add/remove preview entities and refresh the canvas";
}

// Exercise the shipped rectangle creation operation itself, not only its
// custom-entity drag callbacks. Creation should add the storage block and
// outline as one document transaction so one undo restores the full state.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, PythonRectangleCreationIsOneUndoStep) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_::import("sys").attr("path").attr("insert")(
            0, QCoreApplication::applicationDirPath().toStdString());
    }

    lc::ui::MainWindow mw;
    lc::ui::python::MainWindowExecutionScope ownerScope(&mw);
    mw.clearUndoableStack();

    std::size_t initialBlockCount = 0;

    {
        pybind11::gil_scoped_acquire gil;
        auto lcModule = pybind11::module_::import("lc");
        ASSERT_TRUE(lcModule.attr("operation_registry")
                        .contains("PyRectangleOp"));
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        ns["mainWindow"] = pybind11::cast(
            &mw, pybind11::return_value_policy::reference);
        pybind11::exec(
            "initial_block_count = len(mainWindow.cadMdiChild().document().blocks())",
            ns);
        initialBlockCount = ns["initial_block_count"].cast<std::size_t>();
    }

    mw.cliCommand()->runCommand("PYRECTANGLE");
    auto operation = mw.currentOperation();
    ASSERT_FALSE(operation.isNil()) << "PYRECTANGLE did not start PyRectangleOp";
    EXPECT_EQ(operation.getAttr("step").asString(), "AWAIT_P1");
    mw.triggerCoordinateEntered(lc::geo::Coordinate(1.0, 2.0));
    EXPECT_EQ(mw.currentOperation().getAttr("step").asString(), "AWAIT_P2");
    mw.triggerCoordinateEntered(lc::geo::Coordinate(11.0, 10.0));

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        ns["mainWindow"] = pybind11::cast(
            &mw, pybind11::return_value_policy::reference);
        ns["initialBlockCount"] = initialBlockCount;
        pybind11::exec(R"py(
import lc

mdi = mainWindow.cadMdiChild()
doc = mdi.document()
undo = mdi.undoManager()
initial_block_count = initialBlockCount
rectangles = [entity for entity in doc.entityContainer().asVector(32767)
              if isinstance(entity, lc.entity.ScriptCustomEntity)]
assert len(rectangles) == 1, f'creation produced {len(rectangles)} rectangle entities'
assert len(doc.entityContainer().asVector(32767)) == 1, \
    'rectangle creation left unexpected top-level geometry'
block = rectangles[0].displayBlock()
edges = doc.entitiesByBlock(block).asVector(32767)
assert len(edges) == 4, f'expected four rectangle edges, got {len(edges)}'
assert float(block.param('width')) == 10.0
assert float(block.param('height')) == 8.0
assert len(doc.blocks()) == initial_block_count + 1, \
    'rectangle storage block was not added'
assert undo.canUndo() is True, 'rectangle creation did not create an undo entry'

undo.undo()
assert undo.canUndo() is False, 'one undo did not consume the full creation transaction'
assert undo.canRedo() is True, 'undo did not enable redo'
assert len(doc.entityContainer().asVector(32767)) == 0, \
    'one undo did not remove the rectangle geometry'
assert len(doc.blocks()) == initial_block_count, \
    'one undo left the rectangle storage block behind'

undo.redo()
assert undo.canRedo() is False, 'redo did not consume its entry'
rectangles = [entity for entity in doc.entityContainer().asVector(32767)
              if isinstance(entity, lc.entity.ScriptCustomEntity)]
assert len(rectangles) == 1, 'redo did not restore the rectangle entity'
assert len(doc.entitiesByBlock(rectangles[0].displayBlock()).asVector(32767)) == 4, \
    'redo did not restore the four rectangle edges'
assert len(doc.blocks()) == initial_block_count + 1, \
    'redo did not restore the rectangle storage block'
)py",
            ns);
    }

    EXPECT_EQ(mw.luaInterface()->listenerCount("point"), 0u);
    EXPECT_EQ(mw.luaInterface()->listenerCount("mouseMove"), 0u);
}

// Execute the maintained gear example through OperationLoader, its CLI
// command, real number/point events, and the document's undo stack. This
// closes the gap where the plugin was only imported and its builder call
// shape tested without exercising its state machine or generated outline.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, PythonGearOperationCreatesScaledOutline) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_::import("sys").attr("path").attr("insert")(
            0, QCoreApplication::applicationDirPath().toStdString());
    }

    lc::ui::MainWindow mw;
    lc::ui::python::MainWindowExecutionScope ownerScope(&mw);
    mw.clearUndoableStack();

    // MainWindow startup loads the shipped plugin and its decorator must
    // register the command consumed by CliCommand.
    {
        pybind11::gil_scoped_acquire gil;
        auto lcModule = pybind11::module_::import("lc");
        ASSERT_TRUE(lcModule.attr("operation_registry")
                        .contains("PyGearOp"));
    }

    mw.cliCommand()->runCommand("PYGEAR");
    auto operation = mw.currentOperation();
    ASSERT_FALSE(operation.isNil()) << "PYGEAR did not start PyGearOp";
    EXPECT_EQ(operation.getAttr("step").asString(), "AWAIT_TEETH");

    mw.triggerNumberEntered(8.0);
    EXPECT_EQ(mw.currentOperation().getAttr("step").asString(), "AWAIT_PHI");
    mw.triggerNumberEntered(20.0);
    EXPECT_EQ(mw.currentOperation().getAttr("step").asString(), "AWAIT_PC");
    mw.triggerNumberEntered(45.0);
    EXPECT_EQ(mw.currentOperation().getAttr("step").asString(), "AWAIT_ORIGIN");
    mw.triggerCoordinateEntered(lc::geo::Coordinate(10.0, 20.0));
    EXPECT_EQ(mw.currentOperation().getAttr("step").asString(), "AWAIT_SCALE");
    mw.triggerCoordinateEntered(lc::geo::Coordinate(30.0, 20.0));

    // With 8 teeth, 20 degrees, and 45 degrees circular pitch, the sample
    // emits eight vertices per tooth. The scale point is 20 units from the
    // origin, so the pitch-computed outer radius (1.25) becomes 25 units.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        ns["mainWindow"] = pybind11::cast(
            &mw, pybind11::return_value_policy::reference);
        pybind11::exec(R"py(
import math
import lc

mdi = mainWindow.cadMdiChild()
doc = mdi.document()
undo = mdi.undoManager()
entities = doc.entityContainer().asVector(32767)
assert len(entities) == 64, f'expected 64 gear edges, got {len(entities)}'
assert all(isinstance(entity, lc.entity.Line) for entity in entities), \
    'gear outline contains a non-Line entity'
radii = [
    math.hypot(point.x() - 10.0, point.y() - 20.0)
    for line in entities
    for point in (line.start(), line.end())
]
assert abs(min(radii) - 15.0) < 1e-8, f'unexpected root radius: {min(radii)}'
assert abs(max(radii) - 25.0) < 1e-8, f'unexpected outer radius: {max(radii)}'
assert undo.canUndo() is True, 'gear creation did not create an undo entry'

undo.undo()
assert undo.canUndo() is False, 'one undo did not consume the gear transaction'
assert undo.canRedo() is True, 'gear undo did not enable redo'
assert len(doc.entityContainer().asVector(32767)) == 0, \
    'one undo did not remove the complete gear outline'

undo.redo()
redone = doc.entityContainer().asVector(32767)
assert undo.canRedo() is False, 'redo did not consume its entry'
assert len(redone) == 64, f'redo restored {len(redone)} gear edges, expected 64'
)py",
            ns);
    }

    EXPECT_EQ(mw.luaInterface()->listenerCount("number"), 0u);
    EXPECT_EQ(mw.luaInterface()->listenerCount("point"), 0u);
}

// Exercise both remaining maintained operation ports against a real
// MainWindow: PYPOINT must commit one point and release its listeners, and
// PYREMOVE must consume the current selection as one undoable transaction.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, PythonPointAndRemoveOperationsUseRealSelection) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_::import("sys").attr("path").attr("insert")(
            0, QCoreApplication::applicationDirPath().toStdString());
    }

    lc::ui::MainWindow mw;
    lc::ui::python::MainWindowExecutionScope ownerScope(&mw);
    mw.clearUndoableStack();

    mw.cliCommand()->runCommand("PYPOINT");
    auto pointOperation = mw.currentOperation();
    ASSERT_FALSE(pointOperation.isNil()) << "PYPOINT did not start its Python operation";
    EXPECT_EQ(pointOperation.getAttr("step").asString(), "enterPoint");
    mw.triggerCoordinateEntered(lc::geo::Coordinate(4.0, 7.0));
    EXPECT_EQ(mw.luaInterface()->listenerCount("point"), 0u);
    EXPECT_EQ(mw.luaInterface()->listenerCount("mouseMove"), 0u);

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        ns["mainWindow"] = pybind11::cast(
            &mw, pybind11::return_value_policy::reference);
        pybind11::exec(R"py(
import lc

mdi = mainWindow.cadMdiChild()
doc = mdi.document()
undo = mdi.undoManager()
points = doc.entityContainer().asVector(32767)
assert len(points) == 1, f'PYPOINT created {len(points)} entities, expected 1'
assert isinstance(points[0], lc.entity.Point), \
    f'PYPOINT created {type(points[0]).__name__}, expected Point'
assert abs(points[0].x() - 4.0) < 1e-9
assert abs(points[0].y() - 7.0) < 1e-9
assert undo.canUndo() is True, 'PYPOINT did not create an undo entry'
)py",
            ns);
    }

    // Treat the created point as the baseline, then select it using the same
    // MainWindow command used by the GUI before invoking the Python remove op.
    mw.clearUndoableStack();
    mw.selectAll();
    ASSERT_EQ(mw.cadMdiChild()->selection().size(), 1u);
    mw.cliCommand()->runCommand("PYREMOVE");

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        ns["mainWindow"] = pybind11::cast(
            &mw, pybind11::return_value_policy::reference);
        pybind11::exec(R"py(
mdi = mainWindow.cadMdiChild()
doc = mdi.document()
undo = mdi.undoManager()
assert len(doc.entityContainer().asVector(32767)) == 0, \
    'PYREMOVE did not remove the selected point'
assert undo.canUndo() is True, 'PYREMOVE did not create an undo entry'
undo.undo()
assert undo.canUndo() is False, 'one undo did not consume the remove transaction'
assert undo.canRedo() is True, 'undo did not enable redo'
restored = doc.entityContainer().asVector(32767)
assert len(restored) == 1, 'undo did not restore the selected point'
assert abs(restored[0].x() - 4.0) < 1e-9
assert abs(restored[0].y() - 7.0) < 1e-9
undo.redo()
assert undo.canRedo() is False, 'redo did not consume its entry'
assert len(doc.entityContainer().asVector(32767)) == 0, \
    'redo did not restore the removal'
)py",
            ns);
    }
}

#endif  // LC_WITH_PYTHONSCRIPT
