#include "customizetoolbar.h"
#include "ui_customizetoolbar.h"

#include <QToolButton>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QList>

#include "customizeparenttab.h"
#include "customizegrouptab.h"
#include "deleteiconarea.h"
#include "iconlist.h"
#include <iostream>

using namespace lc::ui::widgets;

CustomizeToolbar::CustomizeToolbar(Toolbar* toolbar, QWidget *parent)
    : 
    QDialog(parent),
    _toolbar(toolbar),
    ui(new Ui::CustomizeToolbar)
{
    ui->setupUi(this);

    initializeGroupList();
    initialize();
    initializeParentTab();
}

CustomizeToolbar::~CustomizeToolbar()
{
    delete ui;
}

void CustomizeToolbar::initializeGroupList() {
    QVBoxLayout* verticalLayout = qobject_cast<QVBoxLayout*>(ui->horizontalLayout->itemAt(0)->layout());
    IconList* iconList = new IconList(this);
    verticalLayout->insertWidget(0, iconList);

    // initliaze icon list
    iconList->initialize(_toolbar);

    QSizePolicy sizePolicy1 = iconList->sizePolicy();
    sizePolicy1.setHorizontalStretch(1);
    iconList->setSizePolicy(sizePolicy1);

    QHBoxLayout* optionsGroup = qobject_cast<QHBoxLayout*>(verticalLayout->itemAt(1)->layout());
    optionsGroup->addWidget(new DeleteIconArea());
}

void CustomizeToolbar::initializeParentTab() {
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget());
    tabWidget->setTabsClosable(true);

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
        tabWidget->setCurrentIndex(tabWidget->count() - 2);
    }
}

void CustomizeToolbar::initialize() {
    QList<lc::ui::api::ToolbarTab*> toolbarTabs = _toolbar->tabs();

    for (lc::ui::api::ToolbarTab* toolbarTab : toolbarTabs) {
        addToolbarTab(toolbarTab);
    }
}

void CustomizeToolbar::addToolbarTab(lc::ui::api::ToolbarTab* newTab) {
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget());

    QToolButton* tb = new QToolButton();
    tb->setText("+");
    tabWidget->addTab(new QLabel("Add tabs by pressing \"+\""), QString("Add tab"));
    tabWidget->setTabEnabled(0, false);
    tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, tb);

    connect(tb, &QToolButton::clicked, this, &CustomizeToolbar::addParentTab);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &CustomizeToolbar::parentTabClosed);

    tabWidget->insertTab(tabWidget->count() - 1, new CustomizeParentTab(newTab), QString(newTab->label().c_str()));
}

void CustomizeToolbar::closeEvent(QCloseEvent* e) {
    updateButtons();
    QWidget::closeEvent(e);
}

void CustomizeToolbar::updateButtons() {
    QList<lc::ui::api::ToolbarTab*> toolbarTabs = _toolbar->tabs();

    for (lc::ui::api::ToolbarTab* toolbarTab : toolbarTabs) {
        std::vector<lc::ui::api::ToolbarGroup*> groupsList = toolbarTab->groups();
        bool containsNonButtonGroup = false;

        for (lc::ui::api::ToolbarGroup* toolbarGroup : groupsList) {
            if (!toolbarGroup->nonButtonGroup()) {
                toolbarGroup->clear();
            }
            else {
                containsNonButtonGroup = true;
            }
        }

        toolbarTab->clear();

        // remove tab if it only contains button groups
        if (!containsNonButtonGroup) {
            _toolbar->removeTab(toolbarTab);
        }
    }

    reAddButtons();
}

void CustomizeToolbar::reAddButtons() {
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget());

    QVBoxLayout* verticalLayout = qobject_cast<QVBoxLayout*>(ui->horizontalLayout->itemAt(0)->layout());
    IconList* iconList = qobject_cast<IconList*>(verticalLayout->itemAt(0)->widget());

    // count - 1 because the last "add group" tab should not be considered
    for (int i = 0; i < tabWidget->count() - 1; i++) {
        CustomizeParentTab* parentTab = qobject_cast<CustomizeParentTab*>(tabWidget->widget(i));

        // if tab exists, addTab returns the tab
        lc::ui::api::ToolbarTab* newTab = _toolbar->addTab(parentTab->label().c_str());

        int groupCount = parentTab->count() - 1; // same reason as above for count - 1
        for (int j = 0; j < groupCount; j++) {
            CustomizeGroupTab* groupTab = qobject_cast<CustomizeGroupTab*>(parentTab->widget(j));

            lc::ui::api::ToolbarGroup* newGroup = newTab->addGroup(groupTab->label().c_str());

            // list of buttons in order from group
            QList<QString> buttonNameList = groupTab->buttonNames();

            for (QString& buttonName : buttonNameList) {
                lc::ui::api::ToolbarButton* button = iconList->buttonByName(buttonName);
                if (button != nullptr) {
                    newGroup->addButton(button);
                    button->setVisible(true);
                }
            }
        }
    }
}

void CustomizeToolbar::parentTabClosed(int index) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Remove Tab", "Are you sure you want to remove this tab?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget());
        tabWidget->removeTab(index);
        tabWidget->setCurrentIndex(0);
    }
}
