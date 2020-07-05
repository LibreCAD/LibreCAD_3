#include "customizetoolbar.h"
#include "ui_customizetoolbar.h"

#include <QToolButton>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QList>
#include <QFileDialog>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QFile>

#include "customizegrouptab.h"
#include "deleteiconarea.h"
#include "iconlist.h"
#include <iostream>
#include <string>
#include <sstream>

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
    QWidget* rightSide = ui->horizontalLayout->itemAt(1)->widget();

    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget()->layout()->itemAt(0)->widget());
    tabWidget->setTabsClosable(true);

    QSizePolicy sizePolicy2 = rightSide->sizePolicy();
    sizePolicy2.setHorizontalStretch(2);
    rightSide->setSizePolicy(sizePolicy2);

    tabWidget->setCurrentIndex(0);

    QLayout* buttonsLayout = rightSide->layout()->itemAt(1)->layout();
    QPushButton* defaultButton = qobject_cast<QPushButton*>(buttonsLayout->itemAt(0)->widget());
    QPushButton* loadButton = qobject_cast<QPushButton*>(buttonsLayout->itemAt(1)->widget());

    connect(defaultButton, &QPushButton::clicked, this, &CustomizeToolbar::defaultButtonClicked);
    connect(loadButton, &QPushButton::clicked, this, &CustomizeToolbar::loadToolbarFile);
}

void CustomizeToolbar::addParentTab() {
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add tab"),
                                             tr(""), QLineEdit::Normal,
                                             QDir::home().dirName(), &ok);
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget()->layout()->itemAt(0)->widget());
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
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget()->layout()->itemAt(0)->widget());
    addPlusButton();

    tabWidget->insertTab(tabWidget->count() - 1, new CustomizeParentTab(newTab), QString(newTab->label().c_str()));
}

void CustomizeToolbar::closeEvent(QCloseEvent* e) {
    updateButtons();
    emit customizeWidgetClosed();
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
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget()->layout()->itemAt(0)->widget());

    // count - 1 because the last "add group" tab should not be considered
    for (int i = 0; i < tabWidget->count() - 1; i++) {
        CustomizeParentTab* parentTab = qobject_cast<CustomizeParentTab*>(tabWidget->widget(i));

        // if tab exists, addTab returns the tab
        lc::ui::api::ToolbarTab* newTab = _toolbar->addTab(parentTab->label().c_str());
        
        // to ensure clones are made when duplicate buttons are encountered
        QSet<QString> addedButtonsSet;

        int groupCount = parentTab->count() - 1; // same reason as above for count - 1
        for (int j = 0; j < groupCount; j++) {
            CustomizeGroupTab* groupTab = qobject_cast<CustomizeGroupTab*>(parentTab->widget(j));

            lc::ui::api::ToolbarGroup* newGroup = newTab->addGroup(groupTab->label().c_str(), groupTab->groupWidth());

            // list of buttons in order from group
            QList<QString> buttonNameList = groupTab->buttonNames();

            for (QString& buttonName : buttonNameList) {
                lc::ui::api::ToolbarButton* button = _toolbar->buttonByName(buttonName);
                if (button != nullptr) {

                    if (addedButtonsSet.find(buttonName) != addedButtonsSet.end()) {
                        lc::ui::api::ToolbarButton* clonedButton = button->clone();
                        newGroup->addButton(clonedButton);
                        clonedButton->setVisible(true);
                        continue;
                    }

                    addedButtonsSet.insert(buttonName);
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
        QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget()->layout()->itemAt(0)->widget());
        tabWidget->removeTab(index);
        tabWidget->setCurrentIndex(0);
    }
}

void CustomizeToolbar::generateData(QXmlStreamWriter* streamWriter) {
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget()->layout()->itemAt(0)->widget());
    streamWriter->writeStartElement("toolbar");

    int tabCount = tabWidget->count() - 1;
    // count - 1 because the last "add group" tab should not be considered
    for (int i = 0; i < tabCount; i++) {
        CustomizeParentTab* parentTab = qobject_cast<CustomizeParentTab*>(tabWidget->widget(i));

        int groupCount = parentTab->count() - 1;
        streamWriter->writeStartElement("tab");
        streamWriter->writeAttribute("label", parentTab->label().c_str());

        for (int j = 0; j < groupCount; j++) {
            CustomizeGroupTab* groupTab = qobject_cast<CustomizeGroupTab*>(parentTab->widget(j));

            streamWriter->writeStartElement("group");
            streamWriter->writeAttribute("label", groupTab->label().c_str());
            streamWriter->writeAttribute("width", QString::number(groupTab->groupWidth()));

            // list of buttons in order from group
            QList<QString> buttonNameList = groupTab->buttonNames();

            for (QString& buttonName : buttonNameList) {
                streamWriter->writeTextElement("button", buttonName);
            }
            streamWriter->writeEndElement();
        }
        streamWriter->writeEndElement();
    }

    streamWriter->writeEndElement();
}

void CustomizeToolbar::clearContents() {
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget()->layout()->itemAt(0)->widget());
    int tabCount = tabWidget->count() - 1;

    for (int i = 0; i < tabCount; i++) {
        CustomizeParentTab* parentTab = qobject_cast<CustomizeParentTab*>(tabWidget->widget(i));
        parentTab->clearContents();
    }

    tabWidget->clear();
}

void CustomizeToolbar::readData(QXmlStreamReader* streamReader) {
    clearContents();
    addPlusButton();

    CustomizeParentTab* parentTab = nullptr;
    CustomizeGroupTab* groupTab = nullptr;
    int buttonsCount = 0;
    int groupWidth = 0;

    QString tokenTy = streamReader->name().toString();
    while (tokenTy != "toolbar" && !streamReader->atEnd()) {
        streamReader->readNext();
        tokenTy = streamReader->name().toString();
    }

    while (!streamReader->atEnd()) {
        QString tType = streamReader->name().toString();

        if (!streamReader->isStartElement()) {
            streamReader->readNextStartElement();
            continue;
        }

        if (tType == "tab") {
            QXmlStreamAttributes streamAttributes = streamReader->attributes();
            std::string tabLabel = "New Tab";

            if (streamAttributes.hasAttribute("label")) {
                tabLabel = streamAttributes.value("label").toString().toStdString();
            }

            parentTab = addParentTabManual(tabLabel);
        }

        if (tType == "group") {
            QXmlStreamAttributes streamAttributes = streamReader->attributes();
            std::string groupLabel = "New Group";
            int newWidth = 3;

            if (streamAttributes.hasAttribute("label")) {
                groupLabel = streamAttributes.value("label").toString().toStdString();
            }

            if (streamAttributes.hasAttribute("width")) {
                newWidth = streamAttributes.value("width").toString().toInt();
            }

            if (groupTab != nullptr) {
                groupTab->setWidth(groupWidth, buttonsCount);
                buttonsCount = 0;
            }

            groupWidth = newWidth;
            groupTab = parentTab->addGroupTabManual(groupLabel, groupWidth);
        }

        if (tType == "button") {
            lc::ui::api::ToolbarButton* button = _toolbar->buttonByName(streamReader->readElementText());
            groupTab->addButton(button);
            buttonsCount++;
        }

        streamReader->readNextStartElement();
    }

    groupTab->setWidth(groupWidth, buttonsCount);
}

CustomizeParentTab* CustomizeToolbar::addParentTabManual(std::string tabName) {
    QString text = QString(tabName.c_str());
    CustomizeParentTab* newParentTab = new CustomizeParentTab(text);
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget()->layout()->itemAt(0)->widget());
    if (!text.isEmpty()) {
        tabWidget->insertTab(tabWidget->count() - 1, newParentTab, text);
        tabWidget->setCurrentIndex(tabWidget->count() - 2);
    }

    return newParentTab;
}

void CustomizeToolbar::addPlusButton() {
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget()->layout()->itemAt(0)->widget());

    QToolButton* tb = new QToolButton();
    tb->setText("+");
    tabWidget->addTab(new QLabel("Add tabs by pressing \"+\""), QString("Add tab"));
    tabWidget->setTabEnabled(0, false);
    tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, tb);

    connect(tb, &QToolButton::clicked, this, &CustomizeToolbar::addParentTab);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &CustomizeToolbar::parentTabClosed);
}

void CustomizeToolbar::loadToolbarFile() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open toolbar customization file"), "", tr("XML (*.xml);TEXT (*.txt);All Files (*)"));
    QFile* file = new QFile(fileName);

    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, tr("Unable to open file"), file->errorString());
        return;
    }

    QXmlStreamReader streamReader(file);
    readData(&streamReader);

    file->close();
}

void CustomizeToolbar::defaultButtonClicked() {
    emit defaultSettingsLoad();
}
