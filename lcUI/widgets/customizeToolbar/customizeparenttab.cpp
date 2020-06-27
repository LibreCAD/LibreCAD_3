#include "customizeparenttab.h"
#include "customizegrouptab.h"

#include <QToolButton>
#include <QTabBar>
#include <QLabel>
#include <QInputDialog>
#include <QDir>

using namespace lc::ui::widgets;

CustomizeParentTab::CustomizeParentTab(QString label, QWidget* parent)
    :
      QTabWidget(parent)
{
    QToolButton *tb = new QToolButton();
    tb->setText("+");
    addTab(new QLabel("Add tabs by pressing \"+\""), QString("Add group"));
    setTabEnabled(0, false);
    tabBar()->setTabButton(0, QTabBar::RightSide, tb);

    connect(tb, &QToolButton::clicked, this, &CustomizeParentTab::addGroupTab);

    insertTab(count()-1, new CustomizeGroupTab("Create"), "Create");
    insertTab(count()-1, new CustomizeGroupTab("Modify"), "Modify");
    insertTab(count()-1, new CustomizeGroupTab("Snap"), "Snap");

    setCurrentIndex(0);
}

void CustomizeParentTab::addGroupTab() {
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add tab"),
                                             tr(""), QLineEdit::Normal,
                                             QDir::home().dirName(), &ok);
    if (ok && !text.isEmpty()){
        insertTab(count()-1, new CustomizeGroupTab(text), text);
    }
}
