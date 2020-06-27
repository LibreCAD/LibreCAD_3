#pragma once

#include<QtGui>
#include<QtCore>
#include <QDialog>
#include <QHBoxLayout>
#include <QListView>
#include <QTableView>

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
                CustomizeToolbar(QWidget* parent = nullptr);
                ~CustomizeToolbar();

            public slots:
                void addParentTab();

            private:
                void initializeGroupList();
                void initializeParentTab();

            private:
                Ui::CustomizeToolbar* ui;
            };
        }
    }
}
