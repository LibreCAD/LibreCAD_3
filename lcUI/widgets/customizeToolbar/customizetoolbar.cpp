#include "customizetoolbar.h"
#include "ui_customizetoolbar.h"

#include <QToolButton>
#include <QLabel>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QList>

#include "customizeparenttab.h"
#include "deleteiconarea.h"
#include "iconlist.h"
#include <iostream>

using namespace lc::ui::widgets;

CustomizeToolbar::CustomizeToolbar(Toolbar* toolbar, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CustomizeToolbar)
{
    ui->setupUi(this);

    initializeGroupList(toolbar);
    initialize(toolbar);
    initializeParentTab();
}

CustomizeToolbar::~CustomizeToolbar()
{
    delete ui;
}

void CustomizeToolbar::initializeGroupList(Toolbar* toolbar) {
    QVBoxLayout* verticalLayout = qobject_cast<QVBoxLayout*>(ui->horizontalLayout->itemAt(0)->layout());
    IconList* iconList = new IconList(this);
    verticalLayout->insertWidget(0, iconList);

    // initliaze icon list
    iconList->initialize(toolbar);

    QSizePolicy sizePolicy1 = iconList->sizePolicy();
    sizePolicy1.setHorizontalStretch(1);
    iconList->setSizePolicy(sizePolicy1);

    QHBoxLayout* optionsGroup = qobject_cast<QHBoxLayout*>(verticalLayout->itemAt(1)->layout());
    optionsGroup->addWidget(new DeleteIconArea());
}

void CustomizeToolbar::initializeParentTab() {
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget());

    QSizePolicy sizePolicy2 = tabWidget->sizePolicy();
    sizePolicy2.setHorizontalStretch(2);
    tabWidget->setSizePolicy(sizePolicy2);

    tabWidget->setCurrentIndex(0);
}

void CustomizeToolbar::addParentTab() {
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add tab"),
                                             tr(""), QLineEdit::Normal,
                                             QDir::home().dirName(), &ok);
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget());
    if (ok && !text.isEmpty()){
        tabWidget->insertTab(tabWidget->count()-1, new CustomizeParentTab(text), text);
    }
}

void CustomizeToolbar::initialize(Toolbar* toolbar) {
    QList<lc::ui::api::ToolbarTab*> toolbarTabs = toolbar->tabs();

    for (lc::ui::api::ToolbarTab* toolbarTab : toolbarTabs) {
        addToolbarTab(toolbarTab);
    }
}

void CustomizeToolbar::addToolbarTab(lc::ui::api::ToolbarTab* newTab) {
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget());

    QToolButton* tb = new QToolButton();
    tb->setText("+");
    tabWidget->addTab(new QLabel("Add groups by pressing \"+\""), QString("Add tab"));
    tabWidget->setTabEnabled(0, false);
    tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, tb);

    connect(tb, &QToolButton::clicked, this, &CustomizeToolbar::addParentTab);

    tabWidget->insertTab(tabWidget->count() - 1, new CustomizeParentTab(newTab), QString(newTab->label().c_str()));
}
