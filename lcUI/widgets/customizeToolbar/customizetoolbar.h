#pragma once

#include<QtGui>
#include<QtCore>
#include <QDialog>
#include <QHBoxLayout>
#include <QListView>
#include <QTableView>

#include <widgets/toolbar.h>

QT_BEGIN_NAMESPACE
namespace Ui { class CustomizeToolbar; }
QT_END_NAMESPACE


namespace lc {
    namespace ui {
        namespace widgets {
            class CustomizeToolbar : public QDialog
            {
                Q_OBJECT

            public:
                CustomizeToolbar(Toolbar* toolbar, QWidget* parent = nullptr);

                void initialize();

                void addToolbarTab(lc::ui::api::ToolbarTab* newTab);

                ~CustomizeToolbar();

                void closeEvent(QCloseEvent* event) override;

                void updateButtons();

            public slots:
                void addParentTab();

            private:
                void initializeGroupList();
                void initializeParentTab();

            private:
                Ui::CustomizeToolbar* ui;
                Toolbar* _toolbar;
            };
        }
    }
}
