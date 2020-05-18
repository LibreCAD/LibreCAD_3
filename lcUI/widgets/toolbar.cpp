#include "toolbar.h"
#include "ui_toolbar.h"
#include "widgettitlebar.h"

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

void Toolbar::addTab(const char* name, ToolbarTab* page) {
	ui->tabWidget->addTab(page, tr(name));
}

void Toolbar::removeTab(QWidget *page) {
	if(page == nullptr) {
		return;
	}

	auto index = ui->tabWidget->indexOf(page);
	ui->tabWidget->removeTab(index);
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
    addTab("Quick Access", &quickAccessTab);

    QGroupBox* metaInfoGroup = quickAccessTab.addGroup("Entity properties");
    quickAccessTab.addWidget(metaInfoGroup, linePatternSelect, 0, 0, 1, 1);
    quickAccessTab.addWidget(metaInfoGroup, lineWidthSelect, 0, 1, 1, 1);
    quickAccessTab.addWidget(metaInfoGroup, colorSelect, 0, 2, 1, 1);

    // Add toolbar snap options
    kaguya::State state(luaInterface->luaState());

    state.dostring("snap_op = function(enabled) mainWindow:getCadMdiChild():getSnapManager():setGridSnappable(enabled) end");
    addCheckableButton("", ":/icons/snap_grid.svg", "Snap Options", 0, 0, state["snap_op"], "Snap Grid");

    state.dostring("snap_op = function(enabled) mainWindow:getCadMdiChild():getSnapManager():setIntersectionSnappable(enabled) end");
    addCheckableButton("", ":/icons/snap_intersection.svg", "Snap Options", 0, 1, state["snap_op"], "Snap Intersection");

    state.dostring("snap_op = function(enabled) mainWindow:getCadMdiChild():getSnapManager():setMiddleSnappable(enabled) end");
    addCheckableButton("", ":/icons/snap_middle.svg", "Snap Options", 1, 0, state["snap_op"], "Snap Middle");

    state.dostring("snap_op = function(enabled) mainWindow:getCadMdiChild():getSnapManager():setEntitySnappable(enabled) end");
    addCheckableButton("", ":/icons/snap_entity.svg", "Snap Options", 1, 1, state["snap_op"], "Snap Entity");

    state["snap_op"] = nullptr;
}

void Toolbar::addButton(const char* name, const char* icon, const char* groupBox, int x, int y, kaguya::LuaRef cb, const char* tooltip)
{
    if (quickAccessGroups.find(groupBox) == quickAccessGroups.end()){
        quickAccessGroups[groupBox] = quickAccessTab.addGroup(groupBox);
    }
    QPushButton* button = new QPushButton(name);
    button->setToolTip(tooltip);
    button->setFlat(true);

    if (icon != nullptr) {
        button->setIcon(QIcon(icon));
        button->setIconSize(QSize(24, 24));
    }

    quickAccessTab.addWidget(quickAccessGroups[groupBox], button, x, y, 1, 1);
    luaInterface->luaConnect(button, "pressed()", cb);
}

void Toolbar::addCheckableButton(const char* name, const char* icon, const char* groupBox, int x, int y, kaguya::LuaRef cb, const char* tooltip)
{
    if (quickAccessGroups.find(groupBox) == quickAccessGroups.end())
    {
        quickAccessGroups[groupBox] = quickAccessTab.addGroup(groupBox);
    }
    QPushButton* button = new QPushButton(name);
    button->setToolTip(tooltip);
    button->setFlat(true);
    button->setCheckable(true);

    if (icon != nullptr) {
        button->setIcon(QIcon(icon));
        button->setIconSize(QSize(24, 24));
    }

    quickAccessTab.addWidget(quickAccessGroups[groupBox], button, x, y, 1, 1);
    luaInterface->luaConnect(button, "toggled(bool)", cb);
}

void Toolbar::removeGroupByName(const char* groupName)
{
    if (quickAccessGroups.find(groupName) == quickAccessGroups.end()){
        return;
    }

    quickAccessTab.removeGroup(quickAccessGroups[groupName]);
    quickAccessGroups.remove(groupName);
}
