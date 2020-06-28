#include "customizeparenttab.h"
#include "customizegrouptab.h"

#include <QToolButton>
#include <QTabBar>
#include <QLabel>
#include <QInputDialog>
#include <QDir>

using namespace lc::ui::widgets;

CustomizeParentTab::CustomizeParentTab(lc::ui::api::ToolbarTab* toolbarTab, QWidget* parent)
    :
      QTabWidget(parent)
{
    QToolButton *tb = new QToolButton();
    tb->setText("+");
    addTab(new QLabel("Add tabs by pressing \"+\""), QString("Add group"));
    setTabEnabled(0, false);
    tabBar()->setTabButton(0, QTabBar::RightSide, tb);

    connect(tb, &QToolButton::clicked, this, &CustomizeParentTab::addGroupTab);

    // add groups
    std::vector<lc::ui::api::ToolbarGroup*> groupsList = toolbarTab->groups();

    for (lc::ui::api::ToolbarGroup* toolbarGroup : groupsList) {
        if (!toolbarGroup->nonButtonGroup()) {
            insertTab(count() - 1, new CustomizeGroupTab(toolbarGroup), toolbarGroup->label().c_str());
            /*std::vector<lc::ui::api::ToolbarButton*> toolbarButtons = toolbarGroup->buttons();

            for (lc::ui::api::ToolbarButton* toolbarButton : toolbarButtons)
            {
                std::cout << toolbarButton->label() << std::endl;
            }*/
        }
    }

    setCurrentIndex(0);
}

CustomizeParentTab::CustomizeParentTab(QString label, QWidget* parent) 
    :
    QTabWidget(parent)
{
    QToolButton* tb = new QToolButton();
    tb->setText("+");
    addTab(new QLabel("Add tabs by pressing \"+\""), QString("Add group"));
    setTabEnabled(0, false);
    tabBar()->setTabButton(0, QTabBar::RightSide, tb);

    connect(tb, &QToolButton::clicked, this, &CustomizeParentTab::addGroupTab);
    insertTab(count() - 1, new CustomizeGroupTab("New Group"), "New Group");

    /* TODO ADD RENAME BY DOUBLE CLICKING ON THE GROUP LABEL */

    setCurrentIndex(0);
}

void CustomizeParentTab::addGroupTab() {
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add group"),
                                             tr(""), QLineEdit::Normal,
                                             QDir::home().dirName(), &ok);
    if (ok && !text.isEmpty()){
        insertTab(count()-1, new CustomizeGroupTab(text), text);
    }
}
