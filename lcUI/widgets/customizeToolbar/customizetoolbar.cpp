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
#include <QFile>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/schema.h>

#include "customizegrouptab.h"
#include "deleteiconarea.h"
#include "iconlist.h"
#include <string>
#include <fstream>

using namespace lc::ui::widgets;

CustomizeToolbar::CustomizeToolbar(Toolbar* toolbar, QWidget *parent)
    : 
    QDialog(parent),
    _toolbar(toolbar),
    _saveOnClose(false),
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

    QLayout* buttonsLayout2 = rightSide->layout()->itemAt(2)->layout();
    QPushButton* okButton = qobject_cast<QPushButton*>(buttonsLayout2->itemAt(0)->widget());
    QPushButton* cancelButton = qobject_cast<QPushButton*>(buttonsLayout2->itemAt(1)->widget());

    connect(defaultButton, &QPushButton::clicked, this, &CustomizeToolbar::defaultButtonClicked);
    connect(loadButton, &QPushButton::clicked, this, &CustomizeToolbar::loadToolbarFile);
    connect(okButton, &QPushButton::clicked, this, &CustomizeToolbar::okButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &CustomizeToolbar::cancelButtonClicked);
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

    addPlusButton();
    for (lc::ui::api::ToolbarTab* toolbarTab : toolbarTabs) {
        addToolbarTab(toolbarTab);
    }
}

void CustomizeToolbar::addToolbarTab(lc::ui::api::ToolbarTab* newTab) {
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget()->layout()->itemAt(0)->widget());

    tabWidget->insertTab(tabWidget->count() - 1, new CustomizeParentTab(newTab), QString(newTab->label().c_str()));
}

void CustomizeToolbar::closeEvent(QCloseEvent* e) {
    if (_saveOnClose) {
        updateButtons();
        emit customizeWidgetClosed();
    }
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

void CustomizeToolbar::generateData(rapidjson::Writer<rapidjson::OStreamWrapper>& writer) {
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(ui->horizontalLayout->itemAt(1)->widget()->layout()->itemAt(0)->widget());
    
    writer.StartObject();
    writer.Key("toolbar");
    writer.StartObject();
    writer.Key("tabs");
    writer.StartArray();

    int tabCount = tabWidget->count() - 1;
    // count - 1 because the last "add group" tab should not be considered
    for (int i = 0; i < tabCount; i++) {
        CustomizeParentTab* parentTab = qobject_cast<CustomizeParentTab*>(tabWidget->widget(i));

        int groupCount = parentTab->count() - 1;

        writer.StartObject();
        writer.Key("label");
        writer.String(parentTab->label().c_str());
        writer.String("groups");
        writer.StartArray();

        for (int j = 0; j < groupCount; j++) {
            CustomizeGroupTab* groupTab = qobject_cast<CustomizeGroupTab*>(parentTab->widget(j));

            writer.StartObject();
            writer.Key("label");
            writer.String(groupTab->label().c_str());
            writer.Key("width");
            writer.Int(groupTab->groupWidth());
            writer.String("buttons");
            writer.StartArray();

            // list of buttons in order from group
            QList<QString> buttonNameList = groupTab->buttonNames();

            for (QString& buttonName : buttonNameList) {
                writer.String(buttonName.toStdString().c_str());
            }
            
            writer.EndArray();
            writer.EndObject();
        }
        
        writer.EndArray();
        writer.EndObject();
    }

    writer.EndArray();
    writer.EndObject();
    writer.EndObject();
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

void CustomizeToolbar::readData(rapidjson::Document& document) {
    clearContents();
    addPlusButton();

    const rapidjson::Value& jsonTabs = document["toolbar"]["tabs"];
    rapidjson::SizeType currentTab = 0;

    while (currentTab < jsonTabs.Size()) {
        std::string tabLabel = jsonTabs[currentTab]["label"].GetString();
        CustomizeParentTab* parentTab = parentTab = addParentTabManual(tabLabel);

        const rapidjson::Value& jsonGroups = jsonTabs[currentTab]["groups"];
        rapidjson::SizeType currentGroup = 0;

        while (currentGroup < jsonGroups.Size()) {
            std::string groupLabel = "New Group";
            int newWidth = 3;

            if (jsonGroups[currentGroup].HasMember("label")) {
                groupLabel = jsonGroups[currentGroup]["label"].GetString();
            }

            if (jsonGroups[currentGroup].HasMember("width")) {
                newWidth = jsonGroups[currentGroup]["width"].GetInt();
            }

            CustomizeGroupTab* groupTab = parentTab->addGroupTabManual(groupLabel, newWidth);
            int buttonsCount = 0;
            for (const auto& buttonName : jsonGroups[currentGroup]["buttons"].GetArray()) {
                lc::ui::api::ToolbarButton* button = _toolbar->buttonByName(buttonName.GetString());
                groupTab->addButton(button);

                buttonsCount++;
            }

            groupTab->setWidth(newWidth, buttonsCount);
            currentGroup++;
        }

        currentTab++;
    }
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
    std::ifstream toolbarFile(fileName.toStdString());

    if (toolbarFile.fail()) {
        QMessageBox::information(this, tr("Unable to open file"), "File opening failed");
        return;
    }

    rapidjson::IStreamWrapper isw(toolbarFile);
    rapidjson::Document toolbarDocument;
    toolbarDocument.ParseStream(isw);

    std::ifstream schemaFile("settings_schema.json");
    if (schemaFile.fail()) {
        std::cout << "Schema file not found" << std::endl;
        return;
    }

    rapidjson::IStreamWrapper schemaWrapper(schemaFile);
    rapidjson::Document schemaDocument;
    if (schemaDocument.ParseStream(schemaWrapper).HasParseError()) {
        std::cout << "Schema file is invalid, not json format." << std::endl;
        return;
    }

    rapidjson::SchemaDocument schema(schemaDocument);
    rapidjson::SchemaValidator validator(schema);
    if (!toolbarDocument.Accept(validator)) {
        std::cout << "Document does not satisfy the settings schema." << std::endl;
        return;
    }

    readData(toolbarDocument);

    toolbarFile.close();
}

void CustomizeToolbar::defaultButtonClicked() {
    emit defaultSettingsLoad();
}

void CustomizeToolbar::cancelButtonClicked() {
    _saveOnClose = false;
    close();
}

void CustomizeToolbar::okButtonClicked() {
    _saveOnClose = true;
    close();
}
