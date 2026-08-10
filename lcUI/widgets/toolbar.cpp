#include "toolbar.h"
#include "ui_toolbar.h"
#include "widgettitlebar.h"
#include "guiAPI/toolbarbutton.h"
#include "guiAPI/toolbargroup.h"

// Phase 4 PR-3 — native lambdas replace the snap_op dostring codegen.
// MainWindow → CadMdiChild → SnapManagerImpl direct C++ call path.
#include "mainwindow.h"
#include "cadmdichild.h"
#include <managers/snapmanagerimpl.h>

#include <utility>

using namespace lc::ui::widgets;

Toolbar::Toolbar(lc::ui::LuaInterface* luaInterface, QWidget* parent) :
    QDockWidget(parent),
    ui(new Ui::Toolbar),
    luaInterface(luaInterface) {

    ui->setupUi(this);
    setTitleBarWidget(nullptr);
    setWidget(ui->tabWidget);


    WidgetTitleBar* titleBar = new WidgetTitleBar( "Toolbar", this,
            WidgetTitleBar::TitleBarOptions::HorizontalOnHidden);

    this->setTitleBarWidget(titleBar);
}

Toolbar::~Toolbar() {
    delete ui;
}

lc::ui::api::ToolbarTab* Toolbar::addTab(const char* name) {
    if (_tabs.find(name) != _tabs.end()) {
        return _tabs[name];
    }

    lc::ui::api::ToolbarTab* newTab = new lc::ui::api::ToolbarTab(name);
    addTab(newTab);

    return newTab;
}

void Toolbar::addTab(lc::ui::api::ToolbarTab* newTab) {
    if (newTab == nullptr || _tabs.find(newTab->label().c_str()) != _tabs.end()) {
        return;
    }

    QString name = QString(newTab->label().c_str());
    ui->tabWidget->addTab(newTab, name);
    _tabs[name] = newTab;

    QObject::connect(newTab, SIGNAL(labelChanged(QString, QString)), this, SLOT(setToolbarTabLabel(QString, QString)));
    QObject::connect(newTab, SIGNAL(removeTab(lc::ui::api::ToolbarTab*)), this, SLOT(removeTab(lc::ui::api::ToolbarTab*)));
}

void Toolbar::removeTab(lc::ui::api::ToolbarTab* page) {
    if(page == nullptr) {
        return;
    }

    _tabs.remove(page->label().c_str());

    auto index = ui->tabWidget->indexOf(page);
    ui->tabWidget->removeTab(index);
}

void Toolbar::removeTab(const char* tabName) {
    removeTab(_tabs[tabName]);
}

lc::ui::api::ToolbarTab* Toolbar::tabByName(const char *name) {
    auto nbTab = ui->tabWidget->count();

    for(int i = 0; i < nbTab; i++) {
        if(ui->tabWidget->tabText(i) == name) {
            return qobject_cast<lc::ui::api::ToolbarTab*>(ui->tabWidget->widget(i));
        }
    }

    return nullptr;
}

void Toolbar::closeEvent(QCloseEvent* event)
{
    this->widget()->hide();
    event->ignore();
}

void Toolbar::initializeToolbar(QWidget* linePatternSelect, QWidget* lineWidthSelect, QWidget* colorSelect) {
    lc::ui::api::ToolbarTab* quickAccessTab = addTab("Quick Access");

    // Add metaInfoGroup and the select widgets
    lc::ui::api::ToolbarGroup* metaInfoGroup = new lc::ui::api::ToolbarGroup("Entity properties", 1);
    metaInfoGroup->setNonButtonGroup(true);
    quickAccessTab->addGroup(metaInfoGroup);
    metaInfoGroup->addWidget(linePatternSelect);
    metaInfoGroup->addWidget(lineWidthSelect);
    metaInfoGroup->addWidget(colorSelect);
}

void Toolbar::addSnapOptions() {
    // Phase 4 PR-3 — replaces 4 dostring-codegen sites with native
    // lambdas.  Kill list:
    //   * `snap_op = function(enabled) mainWindow:cadMdiChild():getSnapManager():setGridSnappable(enabled) end`
    //   * `... setIntersectionSnappable(enabled) ...`   (Lua alias for setIntersectionsSnappable)
    //   * `... setMiddleSnappable(enabled) ...`
    //   * `... setEntitySnappable(enabled) ...`
    //   * `state["snap_op"] = nullptr;` (the scratch-global cleanup)
    // The MainWindow parent gives us the CadMdiChild → SnapManagerImpl
    // path directly; the previous chain went through Lua globals just
    // to reach this same C++ method.
    lc::ui::api::ToolbarTab* quickAccessTab = tabByName("Quick Access");
    quickAccessTab->addGroup(new lc::ui::api::ToolbarGroup("Snap Options", 2));

    lc::ui::MainWindow* mainWindow = qobject_cast<lc::ui::MainWindow*>(parentWidget());
    auto snapSetter = [mainWindow](void (lc::viewer::manager::SnapManagerImpl::*fn)(bool),
                                   bool enabled) {
        if (mainWindow && mainWindow->cadMdiChild()) {
            auto sm = mainWindow->cadMdiChild()->getSnapManager();
            if (sm) (sm.get()->*fn)(enabled);
        }
    };

    addButton("SnapGrid",
              ":/icons/snap_grid.svg", "Snap Options",
              lc::scripting::nativeCallback(
                  [snapSetter](const std::vector<lc::scripting::ScriptValue>& args) {
                      snapSetter(&lc::viewer::manager::SnapManagerImpl::setGridSnappable,
                                 !args.empty() && args[0].asBool());
                      return lc::scripting::ScriptValue{};
                  }),
              "Snap Grid", true);

    addButton("SnapIntersection",
              ":/icons/snap_intersection.svg", "Snap Options",
              lc::scripting::nativeCallback(
                  [snapSetter](const std::vector<lc::scripting::ScriptValue>& args) {
                      snapSetter(&lc::viewer::manager::SnapManagerImpl::setIntersectionsSnappable,
                                 !args.empty() && args[0].asBool());
                      return lc::scripting::ScriptValue{};
                  }),
              "Snap Intersection", true);

    addButton("SnapMiddle",
              ":/icons/snap_middle.svg", "Snap Options",
              lc::scripting::nativeCallback(
                  [snapSetter](const std::vector<lc::scripting::ScriptValue>& args) {
                      snapSetter(&lc::viewer::manager::SnapManagerImpl::setMiddleSnappable,
                                 !args.empty() && args[0].asBool());
                      return lc::scripting::ScriptValue{};
                  }),
              "Snap Middle", true);

    addButton("SnapEntity",
              ":/icons/snap_entity.svg", "Snap Options",
              lc::scripting::nativeCallback(
                  [snapSetter](const std::vector<lc::scripting::ScriptValue>& args) {
                      snapSetter(&lc::viewer::manager::SnapManagerImpl::setEntitySnappable,
                                 !args.empty() && args[0].asBool());
                      return lc::scripting::ScriptValue{};
                  }),
              "Snap Entity", true);
}

void Toolbar::addButton(const char* name, const char* icon, const char* groupBox,
                        lc::scripting::ScriptCallback cb,
                        const char* tooltip, bool checkable, const char* tabName)
{
    if (_tabs.find(tabName) == _tabs.end()) {
        addTab(tabName);
    }

    lc::ui::api::ToolbarButton* button =
        new lc::ui::api::ToolbarButton(name, icon, std::move(cb), tooltip, checkable);
    _tabs[tabName]->addButton(button, groupBox);
}

void Toolbar::removeGroupByName(const char* groupName, const char* tabName)
{
    _tabs[tabName]->removeGroup(groupName);
}

void Toolbar::setToolbarTabLabel(QString newLabel, QString oldLabel) {
    lc::ui::api::ToolbarTab* tab = tabByName(oldLabel.toStdString().c_str());
    auto index = ui->tabWidget->indexOf(tab);
    ui->tabWidget->setTabText(index, newLabel);
}

QList<lc::ui::api::ToolbarTab*> Toolbar::tabs() {
    return _tabs.values();
}

void Toolbar::generateButtonsMap() {
    QList<lc::ui::api::ToolbarTab*> toolbarTabs = this->tabs();

    for (lc::ui::api::ToolbarTab* toolbarTab : toolbarTabs) {
        std::vector<lc::ui::api::ToolbarGroup*> groupsList = toolbarTab->groups();

        for (lc::ui::api::ToolbarGroup* toolbarGroup : groupsList) {
            if (!toolbarGroup->nonButtonGroup()) {
                std::vector<lc::ui::api::ToolbarButton*> toolbarButtons = toolbarGroup->buttons();

                for (lc::ui::api::ToolbarButton* toolbarButton : toolbarButtons)
                {
                    QString buttonLabel = QString(toolbarButton->label().c_str());
                    if (_buttonsMap.find(buttonLabel) != _buttonsMap.end()) {
                        continue;
                    }

                    _buttonsMap[buttonLabel] = toolbarButton;
                }
            }
        }
    }
}

lc::ui::api::ToolbarButton* Toolbar::buttonByName(QString buttonName) {
    if (_buttonsMap.find(buttonName) != _buttonsMap.end()) {
        return _buttonsMap[buttonName];
    }
    else {
        return nullptr;
    }
}

QList<QString> Toolbar::buttonNames() const {
    return _buttonsMap.keys();
}

void Toolbar::updateSnapButtons(const viewer::manager::SnapManagerImpl_SPtr snapManager) {
    lc::ui::api::ToolbarButton* snapGridButton = buttonByName("SnapGrid");
    lc::ui::api::ToolbarButton* snapIntersectionButton = buttonByName("SnapIntersection");
    lc::ui::api::ToolbarButton* snapMiddleButton = buttonByName("SnapMiddle");
    lc::ui::api::ToolbarButton* snapEntityButton = buttonByName("SnapEntity");

    snapGridButton->setChecked(snapManager->isGridSnappable());
    snapIntersectionButton->setChecked(snapManager->isIntersectionsSnappable());
    lc::SimpleSnapConstrain snapConstrain = snapManager->snapConstrain();
    snapMiddleButton->setChecked(snapConstrain.hasConstrain(lc::SimpleSnapConstrain::LOGICAL));
    snapEntityButton->setChecked(snapConstrain.hasConstrain(lc::SimpleSnapConstrain::ON_ENTITY));
}
