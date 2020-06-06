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

ToolbarTab* Toolbar::addTab(const char* name) {
    if (tabs.find(name) != tabs.end()) {
        return tabs[name];
    }

    ToolbarTab* newTab = new ToolbarTab(name);
    addTab(newTab);

    return newTab;
}

void Toolbar::addTab(ToolbarTab* newTab) {
    if (newTab == nullptr || tabs.find(newTab->label().c_str()) != tabs.end()) {
        return;
    }
    newTab->setLuaInterface(luaInterface);

    QString name = QString(newTab->label().c_str());
    ui->tabWidget->addTab(newTab, name);
    tabs[name] = newTab;
}

void Toolbar::removeTab(ToolbarTab* page) {
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

ToolbarTab* Toolbar::tabByName(const char *name) {
	auto nbTab = ui->tabWidget->count();

	for(int i = 0; i < nbTab; i++) {
		if(ui->tabWidget->tabText(i) == name) {
			return dynamic_cast<ToolbarTab*>(ui->tabWidget->widget(i));
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
    ToolbarTab* quickAccessTab = addTab("Quick Access");

    // Add metaInfoGroup and the select widgets
    lc::ui::api::ToolbarGroup* metaInfoGroup = new lc::ui::api::ToolbarGroup("Entity properties", 1);
    quickAccessTab->addGroup(metaInfoGroup);
    metaInfoGroup->addWidget(linePatternSelect);
    metaInfoGroup->addWidget(lineWidthSelect);
    metaInfoGroup->addWidget(colorSelect);

    // Add toolbar snap options
    kaguya::State state(luaInterface->luaState());

    quickAccessTab->addGroup(new api::ToolbarGroup("Snap Options", 2));
    state.dostring("snap_op = function(enabled) mainWindow:getCadMdiChild():getSnapManager():setGridSnappable(enabled) end");
    addButton("", ":/icons/snap_grid.svg", "Snap Options", state["snap_op"], "Snap Grid", true);

    state.dostring("snap_op = function(enabled) mainWindow:getCadMdiChild():getSnapManager():setIntersectionSnappable(enabled) end");
    addButton("", ":/icons/snap_intersection.svg", "Snap Options", state["snap_op"], "Snap Intersection", true);

    state.dostring("snap_op = function(enabled) mainWindow:getCadMdiChild():getSnapManager():setMiddleSnappable(enabled) end");
    addButton("", ":/icons/snap_middle.svg", "Snap Options", state["snap_op"], "Snap Middle", true);

    state.dostring("snap_op = function(enabled) mainWindow:getCadMdiChild():getSnapManager():setEntitySnappable(enabled) end");
    addButton("", ":/icons/snap_entity.svg", "Snap Options", state["snap_op"], "Snap Entity", true);

    state["snap_op"] = nullptr;
}

void Toolbar::addButton(const char* name, const char* icon, const char* groupBox, kaguya::LuaRef cb, const char* tooltip, bool checkable, const char* tabName)
{
    if (tabs.find(tabName) == tabs.end()) {
        addTab(tabName);
    }

    lc::ui::api::ToolbarButton* button = new lc::ui::api::ToolbarButton(name, icon, cb, tooltip, checkable);
    button->setLuaInterface(luaInterface);
    tabs[tabName]->addButton(button, groupBox);
}

void Toolbar::removeGroupByName(const char* groupName, const char* tabName)
{
    tabs[tabName]->removeGroup(groupName);
}
