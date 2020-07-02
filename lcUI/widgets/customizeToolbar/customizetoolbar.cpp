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
    addPlusButton();

    tabWidget->insertTab(tabWidget->count() - 1, new CustomizeParentTab(newTab), QString(newTab->label().c_str()));
}

void CustomizeToolbar::closeEvent(QCloseEvent* e) {
    updateButtons();
    std::string toolbar_data = generateData();
    std::cout << toolbar_data << std::endl;
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
        QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget());
        tabWidget->removeTab(index);
        tabWidget->setCurrentIndex(0);
    }
}

std::string CustomizeToolbar::generateData() {
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget());

    std::stringstream dataStream;
    int tabCount = tabWidget->count() - 1;
    dataStream << "<toolbar>" << "\n";

    // count - 1 because the last "add group" tab should not be considered
    for (int i = 0; i < tabCount; i++) {
        CustomizeParentTab* parentTab = qobject_cast<CustomizeParentTab*>(tabWidget->widget(i));

        int groupCount = parentTab->count() - 1;
        dataStream << "<tab label=\"" << parentTab->label() << "\">" << "\n";

        for (int j = 0; j < groupCount; j++) {
            CustomizeGroupTab* groupTab = qobject_cast<CustomizeGroupTab*>(parentTab->widget(j));

            dataStream << "<group label=\"" << groupTab->label() << "\" width=" << groupTab->groupWidth() << ">" << "\n";

            // list of buttons in order from group
            QList<QString> buttonNameList = groupTab->buttonNames();

            for (QString& buttonName : buttonNameList) {
                dataStream << "<button>\"" << buttonName.toStdString() << "\"</button>" << "\n";
            }
            dataStream << "</group>" << "\n";
        }
        dataStream << "</tab>" << "\n";
    }

    dataStream << "</toolbar>" << "\n";
    return dataStream.str();
}

void CustomizeToolbar::clearContents() {
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget());
    int tabCount = tabWidget->count() - 1;

    for (int i = 0; i < tabCount; i++) {
        CustomizeParentTab* parentTab = qobject_cast<CustomizeParentTab*>(tabWidget->widget(i));
        parentTab->clearContents();
    }

    tabWidget->clear();
}

void CustomizeToolbar::readData(std::string data) {
    if (data == "") {
        return;
    }

    clearContents();
    addPlusButton();

    int i = 0;

    CustomizeParentTab* parentTab = nullptr;
    CustomizeGroupTab* groupTab = nullptr;
    int buttonsCount = 0;
    int groupWidth = 0;

    while (i != std::string::npos) {
        std::string tType = tokenType(data, i);

        if (tType == "tab") {
            parentTab = addParentTabManual(tokenLabel(data, i));
        }

        if (tType == "group") {
            if (groupTab != nullptr) {
                groupTab->setWidth(groupWidth, buttonsCount);
                buttonsCount = 0;
            }

            groupWidth = std::stoi(tokenWidth(data, i));
            groupTab = parentTab->addGroupTabManual(tokenLabel(data, i), groupWidth);
        }

        if (tType == "button") {
            lc::ui::api::ToolbarButton* button = _toolbar->buttonByName(QString(tokenButtonText(data, i).c_str()));
            groupTab->addButton(button);
            buttonsCount++;
        }

        advanceToken(data, i);
    }

    groupTab->setWidth(groupWidth, buttonsCount);
}

void CustomizeToolbar::advanceToken(const std::string& data, int& i) const {
    i = data.find("<", i+1);
    while (i != data.size() && data[i + 1] == '/') {
        i = data.find("<", i + 1);
    }
}

std::string CustomizeToolbar::tokenType(const std::string& data, int i) const{
    int endtag = data.find(">", i);
    int space = data.find(" ", i);
    int endIndex = endtag;
    if (space != -1) {
        endIndex = (endtag < space) ? endtag : space;
    }
    endIndex = (endIndex < data.size()) ? endIndex : data.size();
    return data.substr(i + 1, endIndex - i - 1);
}

std::string CustomizeToolbar::tokenLabel(const std::string& data, int i) const{
    int endtag = data.find(">", i);
    int findLabel = data.find("label", i);

    if (findLabel != std::string::npos && findLabel < endtag) {
        int startQuot = data.find("\"", findLabel);
        int endQuot = data.find("\"", startQuot+1);
        return data.substr(startQuot+1, endQuot - startQuot - 1);
    }
    else {
        return "";
    }
}

std::string CustomizeToolbar::tokenWidth(const std::string& data, int i) const{
    int endtag = data.find(">", i);
    int findWidth = data.find("width=", i);

    if (findWidth != std::string::npos && findWidth < endtag) {
        int endtag = data.find(">", findWidth);
        return data.substr(findWidth+6, endtag - findWidth - 6);
    }
    else {
        return "";
    }
}

std::string CustomizeToolbar::tokenButtonText(const std::string& data, int i) const {
    if (tokenType(data, i) != "button") {
        return "";
    }

    int endtag = data.find(">", i);
    int starttag2 = data.find("</", endtag);

    return data.substr(endtag + 2, starttag2 - endtag - 3);
}

std::string CustomizeToolbar::loadFile() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open toolbar customization file"), "", tr("XML (*.xml);TEXT (*.txt);All Files (*)"));

    if (fileName.isEmpty())
    {
        return "";
    }
    else {
        QFile file(fileName);

        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),file.errorString());
            return "";
        }

        QTextStream textIn(&file);
        return textIn.readAll().toStdString();
    }
}

CustomizeParentTab* CustomizeToolbar::addParentTabManual(std::string tabName) {
    QString text = QString(tabName.c_str());
    CustomizeParentTab* newParentTab = new CustomizeParentTab(text);
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget());
    if (!text.isEmpty()) {
        tabWidget->insertTab(tabWidget->count() - 1, newParentTab, text);
        tabWidget->setCurrentIndex(tabWidget->count() - 2);
    }

    return newParentTab;
}

void CustomizeToolbar::addPlusButton() {
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget());

    QToolButton* tb = new QToolButton();
    tb->setText("+");
    tabWidget->addTab(new QLabel("Add tabs by pressing \"+\""), QString("Add tab"));
    tabWidget->setTabEnabled(0, false);
    tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, tb);

    connect(tb, &QToolButton::clicked, this, &CustomizeToolbar::addParentTab);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &CustomizeToolbar::parentTabClosed);
}
