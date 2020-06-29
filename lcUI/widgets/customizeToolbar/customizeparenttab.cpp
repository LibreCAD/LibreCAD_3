#include "customizeparenttab.h"
#include "customizegrouptab.h"

#include <QToolButton>
#include <QTabBar>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include <QDir>

using namespace lc::ui::widgets;

CustomizeParentTab::CustomizeParentTab(lc::ui::api::ToolbarTab* toolbarTab, QWidget* parent)
    :
    _label(toolbarTab->label()),
    QTabWidget(parent)
{
    init();

    // add groups
    std::vector<lc::ui::api::ToolbarGroup*> groupsList = toolbarTab->groups();

    for (lc::ui::api::ToolbarGroup* toolbarGroup : groupsList) {
        if (!toolbarGroup->nonButtonGroup()) {
            insertTab(count() - 1, new CustomizeGroupTab(toolbarGroup), toolbarGroup->label().c_str());
        }
    }

    setCurrentIndex(0);
}

CustomizeParentTab::CustomizeParentTab(QString label, QWidget* parent) 
    :
    _label(label.toStdString()),
    QTabWidget(parent)
{
    init();

    insertTab(count() - 1, new CustomizeGroupTab("New Group"), "New Group");

    /* TODO ADD RENAME BY DOUBLE CLICKING ON THE GROUP LABEL */

    setCurrentIndex(0);
}

void CustomizeParentTab::init() {
    QToolButton* tb = new QToolButton();
    tb->setText("+");
    addTab(new QLabel("Add groups by pressing \"+\""), QString("Add group"));
    setTabEnabled(0, false);
    tabBar()->setTabButton(0, QTabBar::RightSide, tb);
    this->setTabsClosable(true);

    connect(tb, &QToolButton::clicked, this, &CustomizeParentTab::addGroupTab);
    connect(this, &QTabWidget::tabCloseRequested, this, &CustomizeParentTab::groupTabClosed);
}

void CustomizeParentTab::addGroupTab() {
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add group"),
                                             tr(""), QLineEdit::Normal,
                                             QDir::home().dirName(), &ok);
    if (ok && !text.isEmpty()){
        insertTab(count()-1, new CustomizeGroupTab(text), text);
        setCurrentIndex(count() - 2);
    }
}

void CustomizeParentTab::groupTabClosed(int index) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Remove Group", "Are you sure you want to remove this group?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        removeTab(index);
        setCurrentIndex(0);
    }
}

std::string CustomizeParentTab::label() const {
    return _label;
}
