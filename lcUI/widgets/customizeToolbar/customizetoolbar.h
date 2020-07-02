#pragma once

#include<QtGui>
#include<QtCore>
#include <QDialog>
#include <QHBoxLayout>
#include <QListView>
#include <QTableView>

#include <widgets/toolbar.h>
#include "customizeparenttab.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CustomizeToolbar; }
QT_END_NAMESPACE


namespace lc {
    namespace ui {
        namespace widgets {
            /**
            * \brief Toolbar customization widget
            */
            class CustomizeToolbar : public QDialog
            {
                Q_OBJECT

            public:
                /**
                * \brief CustomizeToolbar constructor
                * \param pointer to Toolbar toolbar
                * \param pointer to qwidget parent
                */
                CustomizeToolbar(Toolbar* toolbar, QWidget* parent = nullptr);

                /**
                * \brief Fill widget with existing tabs,groups and buttons
                */
                void initialize();

                /**
                * \brief Add tab corresponding to toolbartab passed in
                * \param pointer to ToolbarTab newTab
                */
                void addToolbarTab(lc::ui::api::ToolbarTab* newTab);

                /**
                * \brief Destructor
                */
                ~CustomizeToolbar();

                /**
                * \brief Overriden close event, to update toolbar on close
                */
                void closeEvent(QCloseEvent* event) override;

                /**
                * \brief Once finished, update buttons in toolbar (called by closeevent)
                */
                void updateButtons();

                /**
                * \brief Generate toolbar ordering data
                */
                std::string generateData();

                /**
                * \brief Read and update buttons from data
                */
                void readData(std::string data);

                /**
                * \brief Clear all data in the customize toolbar widget
                */
                void clearContents();

                /**
                * \brief Add customize parent tab
                * \param string tab name
                */
                CustomizeParentTab* addParentTabManual(std::string tabName);

                /**
                * \brief Add toolbar file
                * \param string file contents
                */
                std::string loadFile();

            public slots:
                /**
                * \brief Slot for add parent tab button in the widget
                */
                void addParentTab();

                /**
                * \brief Close button for parent tab clicked which removes the tab
                */
                void parentTabClosed(int index);

            private:
                void initializeGroupList();

                void initializeParentTab();

                void reAddButtons();

                void addPlusButton();

                void advanceToken(const std::string& data, int& i) const;

                std::string tokenType(const std::string& data, int i) const;

                std::string tokenLabel(const std::string& data, int i) const;

                std::string tokenWidth(const std::string& data, int i) const;

                std::string tokenButtonText(const std::string& data, int i) const;

            private:
                Ui::CustomizeToolbar* ui;
                Toolbar* _toolbar;
            };
        }
    }
}
