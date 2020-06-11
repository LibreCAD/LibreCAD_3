#include "toolbar.h"
#include "ui_toolbar.h"
#include "widgettitlebar.h"
#include "guiAPI/toolbarbutton.h"
#include "guiAPI/toolbargroup.h"

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
    if (tabs.find(name) != tabs.end()) {
        return tabs[name];
    }

    lc::ui::api::ToolbarTab* newTab = new lc::ui::api::ToolbarTab(name);
    addTab(newTab);

    return newTab;
}

void Toolbar::addTab(lc::ui::api::ToolbarTab* newTab) {
    if (newTab == nullptr || tabs.find(newTab->label().c_str()) != tabs.end()) {
        return;
    }
    newTab->enableConnections();

    QString name = QString(newTab->label().c_str());
    ui->tabWidget->addTab(newTab, name);
    tabs[name] = newTab;

    QObject::connect(newTab, SIGNAL(labelChanged(QString, QString)), this, SLOT(setToolbarTabLabel(QString, QString)));
    QObject::connect(newTab, SIGNAL(removeTab(lc::ui::api::ToolbarTab*)), this, SLOT(removeTab(lc::ui::api::ToolbarTab*)));
}

void Toolbar::removeTab(lc::ui::api::ToolbarTab* page) {
	if(page == nullptr) {
		return;
	}

    tabs.remove(page->label().c_str());

	auto index = ui->tabWidget->indexOf(page);
	ui->tabWidget->removeTab(index);
}

void Toolbar::removeTab(const char* tabName) {
    removeTab(tabs[tabName]);
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

void Toolbar::initializeToolbar(QWidget* linePatternSelect, QWidget* lineWidthSelect, QWidget* colorSelect){
    lc::ui::api::ToolbarTab* quickAccessTab = addTab("Quick Access");

    // Add metaInfoGroup and the select widgets
    lc::ui::api::ToolbarGroup* metaInfoGroup = new lc::ui::api::ToolbarGroup("Entity properties", 1);
    quickAccessTab->addGroup(metaInfoGroup);
    metaInfoGroup->addWidget(linePatternSelect);
    metaInfoGroup->addWidget(lineWidthSelect);
    metaInfoGroup->addWidget(colorSelect);
}

void Toolbar::addSnapOptions() {
    // Add toolbar snap options
    lc::ui::api::ToolbarTab* quickAccessTab = tabByName("Quick Access");
    kaguya::State state(luaInterface->luaState());

    quickAccessTab->addGroup(new lc::ui::api::ToolbarGroup("Snap Options", 2));
    state.dostring("snap_op = function(enabled) mainWindow:getCadMdiChild():getSnapManager():setGridSnappable(enabled) end");
    addButton("SnapGrid", ":/icons/snap_grid.svg", "Snap Options", state["snap_op"], "Snap Grid", true);

    state.dostring("snap_op = function(enabled) mainWindow:getCadMdiChild():getSnapManager():setIntersectionSnappable(enabled) end");
    addButton("SnapIntersection", ":/icons/snap_intersection.svg", "Snap Options", state["snap_op"], "Snap Intersection", true);

    state.dostring("snap_op = function(enabled) mainWindow:getCadMdiChild():getSnapManager():setMiddleSnappable(enabled) end");
    addButton("SnapMiddle", ":/icons/snap_middle.svg", "Snap Options", state["snap_op"], "Snap Middle", true);

    state.dostring("snap_op = function(enabled) mainWindow:getCadMdiChild():getSnapManager():setEntitySnappable(enabled) end");
    addButton("SnapEntity", ":/icons/snap_entity.svg", "Snap Options", state["snap_op"], "Snap Entity", true);

    state["snap_op"] = nullptr;
}

void Toolbar::addButton(const char* name, const char* icon, const char* groupBox, kaguya::LuaRef cb, const char* tooltip, bool checkable, const char* tabName)
{
    if (tabs.find(tabName) == tabs.end()) {
        addTab(tabName);
    }

    lc::ui::api::ToolbarButton* button = new lc::ui::api::ToolbarButton(name, icon, cb, tooltip, checkable);
    tabs[tabName]->addButton(button, groupBox);
}

void Toolbar::removeGroupByName(const char* groupName, const char* tabName)
{
    tabs[tabName]->removeGroup(groupName);
}

void Toolbar::setToolbarTabLabel(QString newLabel, QString oldLabel) {
    lc::ui::api::ToolbarTab* tab = tabByName(oldLabel.toStdString().c_str());
    auto index = ui->tabWidget->indexOf(tab);
    ui->tabWidget->setTabText(index, newLabel);
}

void Toolbar::connectToCallbackToolbar(lc::ui::api::ToolbarButton* object, const std::string& signal_name, kaguya::LuaRef& callback) {
    luaInterface->luaConnect(object, signal_name, callback);
}

void Toolbar::disconnectCallbackToolbar(lc::ui::api::ToolbarButton* object, const std::string& signal_name, kaguya::LuaRef& callback) {
    luaInterface->luaDisconnect(object, signal_name, callback);
}
