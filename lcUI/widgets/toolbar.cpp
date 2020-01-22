#include "toolbar.h"
#include "ui_toolbar.h"
#include "widgettitlebar.h"

using namespace lc::ui::widgets;

Toolbar::Toolbar(QWidget *parent) :
	QDockWidget(parent),
	ui(new Ui::Toolbar) {

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