#include "lua/guibridge.h"
#include "luainterface.h"

// Phase 4 PR-2 — Lua adapter for the neutral callback layer.  guibridge
// wraps kaguya::LuaRef → ScriptCallback at every callback-taking API
// site (starting with CliCommand::addCommand; the toolbar/menu/widget
// wraps land in PR-3/PR-4/PR-5).
#include <scriptadapter/luacallback.h>

// Phase 4 PR-8 — OpaquePtr tag identifiers shared with ContextMenuManager.
#include "lua/opaquetags.h"

#include "cadmdichild.h"
#include "documentcanvas.h"
#include "lcadviewer.h"
#include "propertyeditor.h"
#include "widgets/scriptdock.h"
#include "widgets/customizeToolbar/customizetoolbar.h"
#include "widgets/clicommand.h"
#include "widgets/toolbar.h"
#include "widgets/guiAPI/toolbartab.h"
#include "widgets/layers.h"
#include "widgets/guiAPI/menu.h"
#include "widgets/guiAPI/menuitem.h"
#include "widgets/guiAPI/toolbarbutton.h"
#include "widgets/guiAPI/toolbargroup.h"
#include "widgets/guiAPI/dialogwidget.h"
#include "widgets/guiAPI/inputgui.h"
#include "widgets/guiAPI/textgui.h"
#include "widgets/guiAPI/horizontalgroupgui.h"
#include "widgets/guiAPI/buttongui.h"
#include "widgets/guiAPI/checkboxgui.h"
#include "widgets/guiAPI/radiobuttongui.h"
#include "widgets/guiAPI/radiogroupgui.h"
#include "widgets/guiAPI/coordinategui.h"
#include "widgets/guiAPI/anglegui.h"
#include "widgets/guiAPI/slidergui.h"
#include "widgets/guiAPI/comboboxgui.h"
#include "widgets/guiAPI/numbergui.h"
#include "widgets/guiAPI/colorgui.h"
#include "widgets/guiAPI/entitygui.h"
#include "widgets/guiAPI/listgui.h"
#include <drawables/tempentities.h>
#include "mainwindow.h"

using namespace lc;
using namespace viewer;
using namespace ui;
using namespace widgets;
using namespace dialog;
using namespace manager;
using namespace drawable;

void luaOpenGUIBridge(lua_State *L) {
    kaguya::State state(L);
    state["gui"] = kaguya::NewTable();

    // Phase 4 PR-8 — register the api::Menu* OpaquePtr encoder so the
    // Lua adapter can materialize the `menu` argument to
    // `operation:contextMenuOptions(menu)`.  kaguya wraps the raw
    // pointer in the userdata metatable installed by
    // `state["gui"]["Menu"]` in addLuaGUIAPIBindings — which runs below
    // — so Lua-side operations see a normal Menu userdata.
    lc::lua::registerOpaqueEncoder(lc::ui::opaquetag::Menu,
        [](kaguya::State& st, void* ptr) {
            return kaguya::LuaRef(st.state(),
                static_cast<lc::ui::api::Menu*>(ptr));
        });

    // Phase 4 PR-9b — register the CadMdiChild* encoder used by
    // MainWindow's trigger* event slots.  Every payload Map's `widget`
    // entry gets materialized here for Lua listeners.
    lc::lua::registerOpaqueEncoder(lc::ui::opaquetag::CadMdiChild,
        [](kaguya::State& st, void* ptr) {
            return kaguya::LuaRef(st.state(),
                static_cast<lc::ui::CadMdiChild*>(ptr));
        });

    addLCBindings(L);
    addLuaGUIAPIBindings(L);
}

void addLCBindings(lua_State *L) {
    kaguya::State state(L);

    state["lc"]["CadMdiChild"].setClass(kaguya::UserdataMetatable<CadMdiChild>()
    .addStaticFunction("new", []() {
        return new CadMdiChild;
    })
    .addFunction("getSnapManager", &CadMdiChild::getSnapManager)
    .addFunction("cursor", &CadMdiChild::cursor)
    .addFunction("document", &CadMdiChild::document)
    .addFunction("saveFile", &CadMdiChild::saveFile)
    .addFunction("saveAsFile", &CadMdiChild::saveAsFile)
    .addFunction("openFile", &CadMdiChild::openFile)
    .addFunction("selection", &CadMdiChild::selection)
    .addFunction("newDocument", &CadMdiChild::newDocument)
    // Phase 4 PR-10 — setDestroyCallback takes ScriptCallback; wrap LuaRef.
    .addStaticFunction("setDestroyCallback",
        [](CadMdiChild& self, kaguya::LuaRef cb) {
            self.setDestroyCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
    .addFunction("tempEntities", &CadMdiChild::tempEntities)
    .addFunction("undoManager", &CadMdiChild::undoManager)
    .addFunction("viewer", &CadMdiChild::viewer)
    .addFunction("activeLayer", &CadMdiChild::activeLayer)
    .addFunction("metaInfoManager", &CadMdiChild::metaInfoManager)
    .addFunction("getFilename", &CadMdiChild::getFilename)
    //.addFunction("viewerProxy", &CadMdiChild::viewerProxy)
    .addFunction("activeViewport", &CadMdiChild::activeViewport)
                                       );

    //state["lc"]["LCADViewerProxy"].setClass(kaguya::UserdataMetatable<LCADViewerProxy>());
    // not available outside

    state["lc"]["Cursor"].setClass(kaguya::UserdataMetatable<drawable::Cursor>()
                                   .addFunction("position", &drawable::Cursor::position)
                                  );

    state["lc"]["LCADViewer"].setClass(kaguya::UserdataMetatable<LCADViewer>()
                                       .addFunction("autoScale", &LCADViewer::autoScale)
                                       .addFunction("setOperationActive", &LCADViewer::setOperationActive)
                                       .addFunction("docCanvas", &LCADViewer::docCanvas)
                                      );

    state["lc"]["LuaInterface"].setClass(kaguya::UserdataMetatable<LuaInterface>()
                                         .addFunction("pluginList", &LuaInterface::pluginList)
                                         // Phase 4 PR-7 — operation() / setOperation now
                                         // take/return lc::scripting::ScriptObject.  No
                                         // active Lua-side caller reads or writes these
                                         // (only the killed operationloader dostrings did
                                         // — verified by grep against lcUILua/actions and
                                         // lcUILua/plugins*).  The bindings are retained
                                         // for future Lua scripts that want to introspect
                                         // the current operation from Lua; kaguya passes
                                         // the ScriptObject as opaque userdata that Lua
                                         // scripts can only forward, not call methods on.
                                         .addFunction("operation", &LuaInterface::operation)
                                         .addStaticFunction("setOperation",
                                             [](LuaInterface& self, kaguya::LuaRef op) {
                                                 self.setOperation(lc::lua::makeLuaObject(std::move(op)));
                                             })
                                         // Phase 4 PR-9a — registerEvent / triggerEvent are
                                         // overloaded (LuaRef-taking + ScriptValue/ScriptCallback-taking).
                                         // Lua-facing bindings pick the LuaRef overloads
                                         // explicitly.
                                         .addFunction("registerEvent",
                                             static_cast<void(LuaInterface::*)(const std::string&, const kaguya::LuaRef&)>(
                                                 &LuaInterface::registerEvent))
                                         // Phase 5 PR-5.1 — deleteEvent overloaded to accept
                                         // ScriptCallback for `lc.event.deregister`; Lua callers
                                         // continue to see the LuaRef overload.
                                         .addFunction("deleteEvent",
                                             static_cast<void(LuaInterface::*)(const std::string&, const kaguya::LuaRef&)>(
                                                 &LuaInterface::deleteEvent))
                                         .addFunction("triggerEvent",
                                             static_cast<void(LuaInterface::*)(const std::string&, kaguya::LuaRef)>(
                                                 &LuaInterface::triggerEvent))
                                         .addFunction("finishOperation", &LuaInterface::finishOperation)
                                        );

    // Phase 3 PR-3.1 — LuaScript widget was renamed to ScriptDock;
    // bind under the new name AND alias the old name so existing Lua
    // scripts that call `lc.LuaScript(mainWindow):show()` keep working.
    // The alias is a Lua-side table copy, not a fresh class binding —
    // both names resolve to the same metatable.
    state["lc"]["ScriptDock"].setClass(kaguya::UserdataMetatable<widgets::ScriptDock>()
                                      .setConstructors<widgets::ScriptDock(lc::ui::MainWindow*)>()
    .addOverloadedFunctions("show", [](widgets::ScriptDock& self) {
        self.show();
    })
                                     );
    state["lc"]["LuaScript"] = state["lc"]["ScriptDock"];

    state["lc"]["CustomizeToolbar"].setClass(kaguya::UserdataMetatable<widgets::CustomizeToolbar>()
            .setConstructors<widgets::CustomizeToolbar(widgets::Toolbar*)>()
    .addOverloadedFunctions("show", [](widgets::CustomizeToolbar& self) {
        self.show();
    })
                                            );

    state["lc"]["DocumentCanvas"].setClass(kaguya::UserdataMetatable<DocumentCanvas>()
                                           .addFunction("autoScale", &DocumentCanvas::autoScale)
                                           .addFunction("selectPoint", &DocumentCanvas::selectPoint)
                                           .addFunction("removeSelection", &DocumentCanvas::removeSelection)
                                           .addFunction("inverseSelection", &DocumentCanvas::inverseSelection)
                                           .addFunction("selectAll", &DocumentCanvas::selectAll)
                                          );

    // Phase 4 PR-2 — CliCommand::addCommand now takes lc::scripting::ScriptCallback,
    // not kaguya::LuaRef.  Wrap the Lua-side LuaRef into a neutral callback
    // via makeLuaCallback before forwarding.  Lua callers see zero API change.
    state["lc"]["CliCommand"].setClass(kaguya::UserdataMetatable<widgets::CliCommand>()
    .addStaticFunction("addCommand", [](widgets::CliCommand* cliCommand,
                                        const char* name, kaguya::LuaRef cb) {
        return cliCommand->addCommand(name, lc::lua::makeLuaCallback(std::move(cb)));
    })
    .addStaticFunction("write", [](widgets::CliCommand* cliCommand, const char* message) {
        cliCommand->write(message);
    })
    .addFunction("returnText", &widgets::CliCommand::returnText)
    .addFunction("commandActive", &widgets::CliCommand::commandActive)
    .addFunction("runCommand", &widgets::CliCommand::runCommand)
    .addStaticFunction("enableCommand", [](widgets::CliCommand* cliCommand, const char* command) {
        cliCommand->enableCommand(command, true);
    })
    .addStaticFunction("disableCommand", [](widgets::CliCommand* cliCommand, const char* command) {
        cliCommand->enableCommand(command, false);
    })
    .addFunction("isCommandEnabled", &widgets::CliCommand::isCommandEnabled)
    .addFunction("availableCommands", &widgets::CliCommand::availableCommands)
    .addFunction("commandsHistory", &widgets::CliCommand::commandsHistory)
    .addFunction("clear", &widgets::CliCommand::clear)
                                      );

    state["lc"]["Toolbar"].setClass(kaguya::UserdataMetatable<widgets::Toolbar>()
                                    .addFunction("tabByName", &widgets::Toolbar::tabByName)
                                    .addFunction("removeGroupByName", &widgets::Toolbar::removeGroupByName)
                                    .addFunction("updateSnapButtons", &widgets::Toolbar::updateSnapButtons)
                                    .addOverloadedFunctions("addTab", static_cast<api::ToolbarTab*(widgets::Toolbar::*)(const char*)>(&widgets::Toolbar::addTab),
                                            static_cast<void(widgets::Toolbar::*)(api::ToolbarTab*)>(&widgets::Toolbar::addTab))
                                    .addOverloadedFunctions("removeTab", static_cast<void(widgets::Toolbar::*)(api::ToolbarTab*)>(&widgets::Toolbar::removeTab),
                                            static_cast<void(widgets::Toolbar::*)(const char*)>(&widgets::Toolbar::removeTab))
    // Phase 4 PR-3 — Lua-side Toolbar::addButton takes a LuaRef; wrap it
    // into a ScriptCallback before forwarding.
    .addStaticFunction("addButton", [](widgets::Toolbar* toolbar,
                                       const char* name, const char* icon,
                                       const char* group, kaguya::LuaRef cb,
                                       const char* tooltip) {
        toolbar->addButton(name, icon, group,
                           lc::lua::makeLuaCallback(std::move(cb)),
                           tooltip);
    })
                                   );

    state["lc"]["TempEntities"].setClass(kaguya::UserdataMetatable<drawable::TempEntities>()
                                         .addFunction("addEntity", &drawable::TempEntities::addEntity)
                                         .addFunction("removeEntity", &drawable::TempEntities::removeEntity)
                                        );

    state["lc"]["MetaInfoManager"].setClass(kaguya::UserdataMetatable<lc::ui::MetaInfoManager>()
                                            .addFunction("metaInfo", &lc::ui::MetaInfoManager::metaInfo)
                                           );

    state["lc"]["Layers"].setClass(kaguya::UserdataMetatable<widgets::Layers>()
                                   .addFunction("setMdiChild", &widgets::Layers::setMdiChild)
                                   .addFunction("layerByName", &widgets::Layers::layerByName)
                                   .addOverloadedFunctions("addLayer", static_cast<void(widgets::Layers::*)(lc::meta::Layer_CSPtr)>(&widgets::Layers::addLayer),
                                           static_cast<lc::meta::Layer_CSPtr(widgets::Layers::*)(const char*)>(&widgets::Layers::addLayer),
                                           static_cast<lc::meta::Layer_CSPtr(widgets::Layers::*)(const char*, double)>(&widgets::Layers::addLayer),
                                           static_cast<lc::meta::Layer_CSPtr(widgets::Layers::*)(const char*, int, int, int)>(&widgets::Layers::addLayer),
                                           static_cast<lc::meta::Layer_CSPtr(widgets::Layers::*)(const char*, double, int, int, int)>(&widgets::Layers::addLayer),
                                           static_cast<lc::meta::Layer_CSPtr(widgets::Layers::*)(const char*, lc::Color)>(&widgets::Layers::addLayer),
                                           static_cast<lc::meta::Layer_CSPtr(widgets::Layers::*)(const char*, double, lc::Color)>(&widgets::Layers::addLayer))
                                   .addOverloadedFunctions("removeLayer", static_cast<void(widgets::Layers::*)(lc::meta::Layer_CSPtr)>(&widgets::Layers::removeLayer),
                                           static_cast<void(widgets::Layers::*)(const char*)>(&widgets::Layers::removeLayer))
                                   .addOverloadedFunctions("renameLayer", static_cast<void(widgets::Layers::*)(lc::meta::Layer_CSPtr, const char*)>(&widgets::Layers::renameLayer),
                                           static_cast<void(widgets::Layers::*)(const char*, const char*)>(&widgets::Layers::renameLayer))
                                   .addOverloadedFunctions("replaceLayer", static_cast<void(widgets::Layers::*)(lc::meta::Layer_CSPtr, lc::meta::Layer_CSPtr)>(&widgets::Layers::replaceLayerAPI),
                                           static_cast<void(widgets::Layers::*)(const char*, lc::meta::Layer_CSPtr)>(&widgets::Layers::replaceLayerAPI))
                                  );

    state["lc"]["SnapManager"].setClass(kaguya::UserdataMetatable<manager::SnapManagerImpl>()
                                        .addFunction("setGridSnappable", &manager::SnapManagerImpl::setGridSnappable)
                                        .addFunction("setIntersectionSnappable", &manager::SnapManagerImpl::setIntersectionsSnappable)
                                        .addFunction("setMiddleSnappable", &manager::SnapManagerImpl::setMiddleSnappable)
                                        .addFunction("setEntitySnappable", &manager::SnapManagerImpl::setEntitySnappable)
                                       );

    state["lc"]["MainWindow"].setClass(kaguya::UserdataMetatable<lc::ui::MainWindow>()
    // Phase 4 PR-4 — connectMenuItem takes ScriptCallback; wrap LuaRef.
    .addStaticFunction("connectMenuItem",
        [](lc::ui::MainWindow* self, const std::string& itemName, kaguya::LuaRef cb) {
            self->connectMenuItem(itemName, lc::lua::makeLuaCallback(std::move(cb)));
        })
                                       .addFunction("cliCommand", &lc::ui::MainWindow::cliCommand)
                                       .addFunction("cadMdiChild", &lc::ui::MainWindow::cadMdiChild)
                                       .addFunction("toolbar", &lc::ui::MainWindow::toolbar)
                                       .addFunction("layers", &lc::ui::MainWindow::layers)
                                       .addFunction("operationFinished", &lc::ui::MainWindow::operationFinished)
                                       .addFunction("findMenuItem", &lc::ui::MainWindow::findMenuItem)
                                       .addFunction("findMenuItemByObjectName", &lc::ui::MainWindow::findMenuItemByObjectName)
                                       .addFunction("menuByName", &lc::ui::MainWindow::menuByName)
                                       .addFunction("menuByPosition", &lc::ui::MainWindow::menuByPosition)
                                       .addFunction("runCustomizeToolbar", &lc::ui::MainWindow::runCustomizeToolbar)
                                       .addFunction("changeDockLayout", &lc::ui::MainWindow::changeDockLayout)
                                       .addFunction("saveDockLayout", &lc::ui::MainWindow::saveDockLayout)
                                       .addFunction("loadDockLayout", &lc::ui::MainWindow::loadDockLayout)
                                       .addFunction("undo", &lc::ui::MainWindow::undo)
                                       .addFunction("redo", &lc::ui::MainWindow::redo)
                                       .addFunction("selectAll", &lc::ui::MainWindow::selectAll)
                                       .addFunction("selectNone", &lc::ui::MainWindow::selectNone)
                                       .addFunction("invertSelection", &lc::ui::MainWindow::invertSelection)
                                       .addFunction("runLastOperation", &lc::ui::MainWindow::runLastOperation)
                                       .addFunction("currentOperation", &lc::ui::MainWindow::currentOperation)
                                       .addFunction("copySelectedEntities", &lc::ui::MainWindow::copySelectedEntities)
                                       .addFunction("pasteEvent", &lc::ui::MainWindow::pasteEvent)
                                       .addOverloadedFunctions("addMenu", static_cast<lc::ui::api::Menu*(lc::ui::MainWindow::*)(const std::string&)>(&lc::ui::MainWindow::addMenu), static_cast<void(lc::ui::MainWindow::*)(lc::ui::api::Menu*)>(&lc::ui::MainWindow::addMenu))
                                       .addOverloadedFunctions("removeMenu", static_cast<void(lc::ui::MainWindow::*)(const char*)>(&lc::ui::MainWindow::removeMenu), static_cast<void(lc::ui::MainWindow::*)(int)>(&lc::ui::MainWindow::removeMenu))
    // Phase 4 PR-7 — runOperation now takes lc::scripting::ScriptObject.
    // Lua-side callers still send a LuaRef (the operation class table);
    // wrap via makeLuaObject.  Kaguya's overload resolution handles 1-arg
    // and 2-arg call shapes.
    .addOverloadedFunctions("runOperation",
        [](lc::ui::MainWindow& self, kaguya::LuaRef operation, const std::string& init_method) {
            self.runOperation(lc::lua::makeLuaObject(std::move(operation)), init_method);
        },
        [](lc::ui::MainWindow& self, kaguya::LuaRef operation) {
            self.runOperation(lc::lua::makeLuaObject(std::move(operation)));
        })
    // Phase 4 PR-7 — native `runOperationByName` entrypoint the killed
    // dostring codegen used to synthesize.
    .addFunction("runOperationByName", &lc::ui::MainWindow::runOperationByName)
                                      );

    state["lc"]["PropertyEditor"].setClass(kaguya::UserdataMetatable<lc::ui::PropertyEditor>()
                                           .addStaticFunction("GetPropertyEditor", &lc::ui::PropertyEditor::GetPropertyEditor)
                                           .addFunction("propertyChanged", &lc::ui::PropertyEditor::propertyChanged)
                                          );
}

void addLuaGUIAPIBindings(lua_State* L) {
    kaguya::State state(L);

    state["gui"]["Menu"].setClass(kaguya::UserdataMetatable<lc::ui::api::Menu>()
                                  .setConstructors<lc::ui::api::Menu(const char*)>()
                                  .addFunction("label", &lc::ui::api::Menu::label)
                                  .addFunction("setLabel", &lc::ui::api::Menu::setLabel)
                                  .addFunction("position", &lc::ui::api::Menu::position)
                                  .addFunction("setPosition", &lc::ui::api::Menu::setPosition)
                                  .addFunction("remove", &lc::ui::api::Menu::remove)
                                  .addFunction("itemByName", &lc::ui::api::Menu::itemByName)
                                  .addFunction("itemByPosition", &lc::ui::api::Menu::itemByPosition)
                                  .addFunction("menuByName", &lc::ui::api::Menu::menuByName)
                                  .addFunction("menuByPosition", &lc::ui::api::Menu::menuByPosition)
    .addOverloadedFunctions("hide", [](lc::ui::api::Menu& self) {
        self.hide();
    })
    .addOverloadedFunctions("show", [](lc::ui::api::Menu& self) {
        self.show();
    })
    .addOverloadedFunctions("isEnabled", [](lc::ui::api::Menu& self) {
        return self.isEnabled();
    })
    .addOverloadedFunctions("setEnabled", [](lc::ui::api::Menu& self, bool enable) {
        self.setEnabled(enable);
    })
    // Phase 4 PR-4 — the 2-arg addItem(name, cb) takes ScriptCallback;
    // wrap LuaRef.  The other 2 overloads are unchanged.
    .addOverloadedFunctions("addItem", static_cast<void(lc::ui::api::Menu::*)(lc::ui::api::MenuItem*)>(&lc::ui::api::Menu::addItem),
                            static_cast<lc::ui::api::MenuItem * (lc::ui::api::Menu::*)(const char*)>(&lc::ui::api::Menu::addItem),
        [](lc::ui::api::Menu& self, const char* label, kaguya::LuaRef cb) {
            return self.addItem(label, lc::lua::makeLuaCallback(std::move(cb)));
        })
    .addOverloadedFunctions("removeItem", static_cast<void(lc::ui::api::Menu::*)(lc::ui::api::MenuItem*)>(&lc::ui::api::Menu::removeItem), static_cast<void(lc::ui::api::Menu::*)(const char*)>(&lc::ui::api::Menu::removeItem))
    .addOverloadedFunctions("removeMenu", static_cast<void(lc::ui::api::Menu::*)(lc::ui::api::Menu*)>(&lc::ui::api::Menu::removeMenu), static_cast<void(lc::ui::api::Menu::*)(const char*)>(&lc::ui::api::Menu::removeMenu))
    .addOverloadedFunctions("addMenu", static_cast<lc::ui::api::Menu * (lc::ui::api::Menu::*)(const char*)>(&lc::ui::api::Menu::addMenu), static_cast<void(lc::ui::api::Menu::*)(lc::ui::api::Menu*)>(&lc::ui::api::Menu::addMenu))
                                 );

    // Phase 4 PR-4 — MenuItem's callback-taking constructor is exposed
    // as a static factory `gui.MenuItem.new(label, cb)`, same idiom as
    // ToolbarButton in PR-3.  Non-callback constructor stays direct.
    // addCallback + addCheckedCallback take ScriptCallback; wrap LuaRef.
    state["gui"]["MenuItem"].setClass(kaguya::UserdataMetatable<lc::ui::api::MenuItem>()
                                      .setConstructors<lc::ui::api::MenuItem(const char*)>()
    .addStaticFunction("new",
        [](const char* label, kaguya::LuaRef cb) {
            return new lc::ui::api::MenuItem(label, lc::lua::makeLuaCallback(std::move(cb)));
        })
                                      .addFunction("label", &lc::ui::api::MenuItem::label)
                                      .addFunction("setLabel", &lc::ui::api::MenuItem::setLabel)
                                      .addFunction("position", &lc::ui::api::MenuItem::position)
                                      .addFunction("setPosition", &lc::ui::api::MenuItem::setPosition)
                                      .addFunction("remove", &lc::ui::api::MenuItem::remove)
                                      .addFunction("removeCallback", &lc::ui::api::MenuItem::removeCallback)
                                      .addFunction("setCheckable", &lc::ui::api::MenuItem::setCheckable)
                                      .addFunction("setChecked", &lc::ui::api::MenuItem::setChecked)
    .addStaticFunction("addCheckedCallback",
        [](lc::ui::api::MenuItem& self, kaguya::LuaRef cb) {
            self.addCheckedCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
    .addOverloadedFunctions("hide", [](lc::ui::api::MenuItem& self) {
        self.hide();
    })
    .addOverloadedFunctions("show", [](lc::ui::api::MenuItem& self) {
        self.show();
    })
    .addOverloadedFunctions("isEnabled", [](lc::ui::api::MenuItem& self) {
        return self.isEnabled();
    })
    .addOverloadedFunctions("setEnabled", [](lc::ui::api::MenuItem& self, bool enable) {
        self.setEnabled(enable);
    })
    .addOverloadedFunctions("addCallback",
        [](lc::ui::api::MenuItem& self, kaguya::LuaRef cb) {
            self.addCallback(lc::lua::makeLuaCallback(std::move(cb)));
        },
        [](lc::ui::api::MenuItem& self, const char* name, kaguya::LuaRef cb) {
            self.addCallback(name, lc::lua::makeLuaCallback(std::move(cb)));
        })
                                     );

    state["gui"]["ToolbarTab"].setClass(kaguya::UserdataMetatable<api::ToolbarTab>()
                                        .setConstructors<api::ToolbarTab(const char*)>()
                                        .addFunction("addButton", &api::ToolbarTab::addButton)
                                        .addFunction("buttonByText", &api::ToolbarTab::buttonByText)
                                        .addFunction("groupByName", &api::ToolbarTab::groupByName)
                                        .addFunction("label", &api::ToolbarTab::label)
                                        .addFunction("setLabel", &api::ToolbarTab::setLabel)
                                        .addFunction("groups", &api::ToolbarTab::groups)
                                        .addFunction("remove", &api::ToolbarTab::remove)
    .addOverloadedFunctions("enable", [](lc::ui::api::ToolbarTab& self) {
        self.setEnabled(true);
    })
    .addOverloadedFunctions("disable", [](lc::ui::api::ToolbarTab& self) {
        self.setEnabled(false);
    })
    .addOverloadedFunctions("removeGroup", static_cast<void(api::ToolbarTab::*)(lc::ui::api::ToolbarGroup*)>(&api::ToolbarTab::removeGroup),
                            static_cast<void(api::ToolbarTab::*)(const char*)>(&api::ToolbarTab::removeGroup))
    .addOverloadedFunctions("addGroup", static_cast<void(api::ToolbarTab::*)(api::ToolbarGroup*)>(&api::ToolbarTab::addGroup),
                            static_cast<api::ToolbarGroup * (api::ToolbarTab::*)(const char*, int)>(&api::ToolbarTab::addGroup),
    [](api::ToolbarTab& self, const char* name) {
        return self.addGroup(name);
    })
                                       );

    // Phase 4 PR-3 — ToolbarButton's Lua-facing constructors and
    // addCallback overloads take a LuaRef; wrap into ScriptCallback
    // before forwarding.  The 3 callback-taking constructor arities are
    // exposed as `gui.ToolbarButton.new(...)` static factory shapes
    // (kaguya's setConstructors can't emit lambdas per arity, but
    // `.addStaticFunction("new", ...)` can — Lua callers can either use
    // the 3-arg + optional args new() variants or the ctor overloads
    // that don't take a callback).  Non-callback ctors stay unchanged.
    state["gui"]["ToolbarButton"].setClass(kaguya::UserdataMetatable<lc::ui::api::ToolbarButton>()
                                           .setConstructors<lc::ui::api::ToolbarButton(const char*, const char*),
                                           lc::ui::api::ToolbarButton(const char*, const char*, const char*),
                                           lc::ui::api::ToolbarButton(const char*, const char*, const char*, bool)>()
    .addStaticFunction("new", [](const char* label, const char* icon, kaguya::LuaRef cb) {
        return new lc::ui::api::ToolbarButton(label, icon,
            lc::lua::makeLuaCallback(std::move(cb)));
    })
    .addStaticFunction("newWithTooltip",
        [](const char* label, const char* icon, kaguya::LuaRef cb, const char* tooltip) {
            return new lc::ui::api::ToolbarButton(label, icon,
                lc::lua::makeLuaCallback(std::move(cb)), tooltip);
        })
    .addStaticFunction("newCheckable",
        [](const char* label, const char* icon, kaguya::LuaRef cb,
           const char* tooltip, bool checkable) {
            return new lc::ui::api::ToolbarButton(label, icon,
                lc::lua::makeLuaCallback(std::move(cb)), tooltip, checkable);
        })
                                           .addFunction("label", &lc::ui::api::ToolbarButton::label)
                                           .addFunction("setLabel", &lc::ui::api::ToolbarButton::setLabel)
                                           .addFunction("setTooltip", &lc::ui::api::ToolbarButton::setTooltip)
                                           .addFunction("removeCallback", &lc::ui::api::ToolbarButton::removeCallback)
                                           .addFunction("remove", &lc::ui::api::ToolbarButton::remove)
    .addOverloadedFunctions("hide", [](lc::ui::api::ToolbarButton& self) {
        self.hide();
    })
    .addOverloadedFunctions("show", [](lc::ui::api::ToolbarButton& self) {
        self.show();
    })
    .addOverloadedFunctions("isEnabled", [](lc::ui::api::ToolbarButton& self) {
        return self.isEnabled();
    })
    .addOverloadedFunctions("enable", [](lc::ui::api::ToolbarButton& self) {
        self.setEnabled(true);
    })
    .addOverloadedFunctions("disable", [](lc::ui::api::ToolbarButton& self) {
        self.setEnabled(false);
    })
    .addOverloadedFunctions("addCallback",
        [](lc::ui::api::ToolbarButton& self, kaguya::LuaRef cb) {
            self.addCallback(lc::lua::makeLuaCallback(std::move(cb)));
        },
        [](lc::ui::api::ToolbarButton& self, const char* name, kaguya::LuaRef cb) {
            self.addCallback(name, lc::lua::makeLuaCallback(std::move(cb)));
        })
                                          );

    state["gui"]["ToolbarGroup"].setClass(kaguya::UserdataMetatable<lc::ui::api::ToolbarGroup>()
                                          .setConstructors<lc::ui::api::ToolbarGroup(const char*), lc::ui::api::ToolbarGroup(const char*, int)>()
                                          .addFunction("label", &lc::ui::api::ToolbarGroup::label)
                                          .addFunction("setLabel", &lc::ui::api::ToolbarGroup::setLabel)
                                          .addFunction("buttonByName", &lc::ui::api::ToolbarGroup::buttonByName)
                                          .addFunction("buttons", &lc::ui::api::ToolbarGroup::buttons)
                                          .addFunction("remove", &lc::ui::api::ToolbarGroup::remove)
                                          .addFunction("setWidth", &lc::ui::api::ToolbarGroup::setWidth)
                                          .addFunction("width", &lc::ui::api::ToolbarGroup::width)
    .addOverloadedFunctions("hide", [](lc::ui::api::ToolbarGroup& self) {
        self.hide();
    })
    .addOverloadedFunctions("show", [](lc::ui::api::ToolbarGroup& self) {
        self.show();
    })
    .addOverloadedFunctions("enable", [](lc::ui::api::ToolbarGroup& self) {
        self.setEnabled(true);
    })
    .addOverloadedFunctions("disable", [](lc::ui::api::ToolbarGroup& self) {
        self.setEnabled(false);
    })
    // Phase 4 PR-3 — the callback-taking addButton overload takes a
    // LuaRef on the Lua side; wrap into ScriptCallback before forwarding.
    .addOverloadedFunctions("addButton", static_cast<void(lc::ui::api::ToolbarGroup::*)(lc::ui::api::ToolbarButton*)>(&lc::ui::api::ToolbarGroup::addButton),
                            static_cast<lc::ui::api::ToolbarButton * (lc::ui::api::ToolbarGroup::*)(const char*, const char*)>(&lc::ui::api::ToolbarGroup::addButton),
        [](lc::ui::api::ToolbarGroup& self, const char* name, const char* icon,
           kaguya::LuaRef cb) {
            return self.addButton(name, icon, lc::lua::makeLuaCallback(std::move(cb)));
        })
    .addOverloadedFunctions("removeButton", static_cast<void(lc::ui::api::ToolbarGroup::*)(lc::ui::api::ToolbarButton*)>(&lc::ui::api::ToolbarGroup::removeButton),
                            static_cast<void(lc::ui::api::ToolbarGroup::*)(const char*)>(&lc::ui::api::ToolbarGroup::removeButton))
                                         );

    state["gui"]["InputGUIContainer"].setClass(kaguya::UserdataMetatable<lc::ui::api::InputGUIContainer>()
            .addFunction("inputWidgets", &lc::ui::api::InputGUIContainer::inputWidgets)
            // Phase 4 PR-5b — InputGUIContainer::addFinishCallback now
            // takes a neutral ScriptCallback; wrap Lua callers' LuaRef.
            .addStaticFunction("addFinishCallback",
                [](lc::ui::api::InputGUIContainer& self, kaguya::LuaRef cb) {
                    self.addFinishCallback(lc::lua::makeLuaCallback(std::move(cb)));
                })
            .addFunction("keys", &lc::ui::api::InputGUIContainer::keys)
            .addFunction("addWidget", &lc::ui::api::InputGUIContainer::addWidget)
                                              );

    state["gui"]["DialogWidget"].setClass(kaguya::UserdataMetatable<lc::ui::api::DialogWidget, lc::ui::api::InputGUIContainer>()
                                          .setConstructors<lc::ui::api::DialogWidget(const std::string&, lc::ui::MainWindow*)>()
                                          .addFunction("setFinishButton", &lc::ui::api::DialogWidget::setFinishButton)
    .addOverloadedFunctions("enable", [](lc::ui::api::DialogWidget& self) {
        self.setEnabled(true);
    })
    .addOverloadedFunctions("disable", [](lc::ui::api::DialogWidget& self) {
        self.setEnabled(false);
    })
    .addFunction("addWidget", &lc::ui::api::DialogWidget::addWidget)
                                         );

    state["gui"]["InputGUI"].setClass(kaguya::UserdataMetatable<lc::ui::api::InputGUI>()
                                      .addFunction("label", &lc::ui::api::InputGUI::label)
                                      .addFunction("setLabel", &lc::ui::api::InputGUI::setLabel)
                                      .addFunction("key", &lc::ui::api::InputGUI::key)
    .addOverloadedFunctions("enable", [](lc::ui::api::InputGUI& self) {
        self.setEnabled(true);
    })
    .addOverloadedFunctions("disable", [](lc::ui::api::InputGUI& self) {
        self.setEnabled(false);
    })
                                     );

    state["gui"]["Text"].setClass(kaguya::UserdataMetatable<lc::ui::api::TextGUI, lc::ui::api::InputGUI>()
                                  .setConstructors<lc::ui::api::TextGUI(std::string)>()
                                  .addFunction("value", &lc::ui::api::TextGUI::value)
                                  .addFunction("setValue", &lc::ui::api::TextGUI::setValue)
    // Phase 4 PR-5a — scalar widget callbacks take ScriptCallback; wrap LuaRef.
    .addStaticFunction("addFinishCallback",
        [](lc::ui::api::TextGUI& self, kaguya::LuaRef cb) {
            self.addFinishCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
    .addStaticFunction("addOnChangeCallback",
        [](lc::ui::api::TextGUI& self, kaguya::LuaRef cb) {
            self.addOnChangeCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
                                 );

    state["gui"]["Button"].setClass(kaguya::UserdataMetatable<lc::ui::api::ButtonGUI, lc::ui::api::InputGUI>()
                                    .setConstructors<lc::ui::api::ButtonGUI(std::string)>()
                                    .addFunction("setLabel", &lc::ui::api::ButtonGUI::setLabel)
    // Phase 4 PR-5a — wrap LuaRef -> ScriptCallback.
    .addStaticFunction("addCallback",
        [](lc::ui::api::ButtonGUI& self, kaguya::LuaRef cb) {
            self.addCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
                                   );

    state["gui"]["CheckBox"].setClass(kaguya::UserdataMetatable<lc::ui::api::CheckBoxGUI, lc::ui::api::InputGUI>()
                                      .setConstructors<lc::ui::api::CheckBoxGUI(std::string), lc::ui::api::CheckBoxGUI(std::string, bool)>()
                                      .addFunction("setLabel", &lc::ui::api::CheckBoxGUI::setLabel)
    // Phase 4 PR-5a — wrap LuaRef -> ScriptCallback.
    .addStaticFunction("addCallback",
        [](lc::ui::api::CheckBoxGUI& self, kaguya::LuaRef cb) {
            self.addCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
                                      .addFunction("value", &lc::ui::api::CheckBoxGUI::value)
                                      .addFunction("setValue", &lc::ui::api::CheckBoxGUI::setValue)
                                     );

    state["gui"]["RadioButton"].setClass(kaguya::UserdataMetatable<lc::ui::api::RadioButtonGUI>()
                                         .setConstructors<lc::ui::api::RadioButtonGUI(std::string)>()
                                         .addFunction("label", &lc::ui::api::RadioButtonGUI::label)
                                         .addFunction("setLabel", &lc::ui::api::RadioButtonGUI::setLabel)
    // Phase 4 PR-5a — wrap LuaRef -> ScriptCallback.
    .addStaticFunction("addCallback",
        [](lc::ui::api::RadioButtonGUI& self, kaguya::LuaRef cb) {
            self.addCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
    .addOverloadedFunctions("checked", [](lc::ui::api::RadioButtonGUI& self) {
        self.isChecked();
    })
    .addOverloadedFunctions("setChecked", [](lc::ui::api::RadioButtonGUI& self, bool check) {
        self.setChecked(check);
    })
                                        );

    state["gui"]["HorizontalGroup"].setClass(kaguya::UserdataMetatable<lc::ui::api::HorizontalGroupGUI, lc::ui::api::InputGUI>()
            .setConstructors<lc::ui::api::HorizontalGroupGUI(std::string)>()
    .addOverloadedFunctions("addWidget", [](lc::ui::api::HorizontalGroupGUI& self, const std::string& key, lc::ui::api::ButtonGUI* newButton) {
        self.addWidget(key, newButton);
    },
    [](lc::ui::api::HorizontalGroupGUI& self, const std::string& key, lc::ui::api::InputGUI* newGUI) {
        self.addWidget(key, newGUI);
    },
    [](lc::ui::api::HorizontalGroupGUI& self, const std::string& key, lc::ui::api::CheckBoxGUI* checkGUI) {
        self.addWidget(key, checkGUI);
    })
                                            );

    state["gui"]["RadioGroup"].setClass(kaguya::UserdataMetatable<lc::ui::api::RadioGroupGUI, lc::ui::api::InputGUI>()
                                        .setConstructors<lc::ui::api::RadioGroupGUI(std::string)>()
                                        .addFunction("addButton", &lc::ui::api::RadioGroupGUI::addButton)
                                       );

    state["gui"]["Coordinate"].setClass(kaguya::UserdataMetatable<lc::ui::api::CoordinateGUI, lc::ui::api::InputGUI>()
                                        .setConstructors<lc::ui::api::CoordinateGUI(std::string)>()
    // Phase 4 PR-5a — wrap LuaRef -> ScriptCallback.
    .addStaticFunction("addFinishCallback",
        [](lc::ui::api::CoordinateGUI& self, kaguya::LuaRef cb) {
            self.addFinishCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
    .addStaticFunction("addOnChangeCallback",
        [](lc::ui::api::CoordinateGUI& self, kaguya::LuaRef cb) {
            self.addOnChangeCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
                                        .addFunction("value", &lc::ui::api::CoordinateGUI::value)
                                        .addFunction("setValue", &lc::ui::api::CoordinateGUI::setValue)
                                       );

    state["gui"]["Angle"].setClass(kaguya::UserdataMetatable<lc::ui::api::AngleGUI, lc::ui::api::InputGUI>()
                                   .setConstructors<lc::ui::api::AngleGUI(std::string)>()
                                   .addFunction("toDegrees", &lc::ui::api::AngleGUI::toDegrees)
                                   .addFunction("toRadians", &lc::ui::api::AngleGUI::toRadians)
    // Phase 4 PR-5a — wrap LuaRef -> ScriptCallback.
    .addStaticFunction("addFinishCallback",
        [](lc::ui::api::AngleGUI& self, kaguya::LuaRef cb) {
            self.addFinishCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
    .addStaticFunction("addOnChangeCallback",
        [](lc::ui::api::AngleGUI& self, kaguya::LuaRef cb) {
            self.addOnChangeCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
                                   .addFunction("value", &lc::ui::api::AngleGUI::value)
                                   .addFunction("setValue", &lc::ui::api::AngleGUI::setValue)
                                  );

    state["gui"]["Slider"].setClass(kaguya::UserdataMetatable<lc::ui::api::SliderGUI, lc::ui::api::InputGUI>()
                                    .setConstructors<lc::ui::api::SliderGUI(std::string), lc::ui::api::SliderGUI(std::string, int minVal, int maxVal)>()
    // Phase 4 PR-5a — wrap LuaRef -> ScriptCallback.
    .addStaticFunction("addCallback",
        [](lc::ui::api::SliderGUI& self, kaguya::LuaRef cb) {
            self.addCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
                                    .addFunction("setLabel", &lc::ui::api::SliderGUI::setLabel)
                                    .addFunction("value", &lc::ui::api::SliderGUI::value)
                                    .addFunction("setValue", &lc::ui::api::SliderGUI::setValue)
                                   );

    state["gui"]["ComboBox"].setClass(kaguya::UserdataMetatable<lc::ui::api::ComboBoxGUI, lc::ui::api::InputGUI>()
                                      .setConstructors<lc::ui::api::ComboBoxGUI(std::string)>()
    // Phase 4 PR-5a — wrap LuaRef -> ScriptCallback.
    .addStaticFunction("addCallback",
        [](lc::ui::api::ComboBoxGUI& self, kaguya::LuaRef cb) {
            self.addCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
                                      .addFunction("setLabel", &lc::ui::api::ComboBoxGUI::setLabel)
                                      .addFunction("value", &lc::ui::api::ComboBoxGUI::value)
    .addOverloadedFunctions("addItem", [](lc::ui::api::ComboBoxGUI& self, const std::string& item) {
        self.addItem(item);
    },
    [](lc::ui::api::ComboBoxGUI& self, const std::string& item, int index) {
        self.addItem(item, index);
    })
    .addOverloadedFunctions("setValue", static_cast<void(lc::ui::api::ComboBoxGUI::*)(const std::string&)>(&lc::ui::api::ComboBoxGUI::setValue),
                            static_cast<void(lc::ui::api::ComboBoxGUI::*)(int)>(&lc::ui::api::ComboBoxGUI::setValue))
                                     );

    state["gui"]["Number"].setClass(kaguya::UserdataMetatable<lc::ui::api::NumberGUI, lc::ui::api::InputGUI>()
                                    .setConstructors<lc::ui::api::NumberGUI(std::string)>()
    // Phase 4 PR-5a — wrap LuaRef -> ScriptCallback.
    .addStaticFunction("addCallback",
        [](lc::ui::api::NumberGUI& self, kaguya::LuaRef cb) {
            self.addCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
                                    .addFunction("setLabel", &lc::ui::api::NumberGUI::setLabel)
                                    .addFunction("value", &lc::ui::api::NumberGUI::value)
                                    .addFunction("setValue", &lc::ui::api::NumberGUI::setValue)
                                   );

    state["gui"]["ColorPicker"].setClass(kaguya::UserdataMetatable<lc::ui::api::ColorGUI, lc::ui::api::InputGUI>()
                                         .setConstructors<lc::ui::api::ColorGUI(std::string)>()
                                         .addFunction("value", &lc::ui::api::ColorGUI::value)
                                         .addFunction("setValue", &lc::ui::api::ColorGUI::setValue)
    // Phase 4 PR-5a — wrap LuaRef -> ScriptCallback.
    .addStaticFunction("addCallback",
        [](lc::ui::api::ColorGUI& self, kaguya::LuaRef cb) {
            self.addCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
                                        );

    state["gui"]["EntityPicker"].setClass(kaguya::UserdataMetatable<lc::ui::api::EntityGUI, lc::ui::api::InputGUI>()
                                          .setConstructors<lc::ui::api::EntityGUI(std::string)>()
                                          .addFunction("value", &lc::ui::api::EntityGUI::value)
                                          .addFunction("setValue", &lc::ui::api::EntityGUI::setValue)
                                          .addFunction("addEntity", &lc::ui::api::EntityGUI::addEntity)
    // Phase 4 PR-5a — wrap LuaRef -> ScriptCallback.
    .addStaticFunction("addCallback",
        [](lc::ui::api::EntityGUI& self, kaguya::LuaRef cb) {
            self.addCallback(lc::lua::makeLuaCallback(std::move(cb)));
        })
                                         );

    state["gui"]["List"].setClass(kaguya::UserdataMetatable<lc::ui::api::ListGUI, lc::ui::api::InputGUI>()
                                  .setConstructors<lc::ui::api::ListGUI(std::string)>()
                                  .addFunction("addItem", &lc::ui::api::ListGUI::addItem)
                                  .addOverloadedFunctions("setListType", static_cast<void(lc::ui::api::ListGUI::*)(const std::string&)>(&lc::ui::api::ListGUI::setListType))
                                 );
}
