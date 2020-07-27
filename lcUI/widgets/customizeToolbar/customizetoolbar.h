#pragma once

#include<QtGui>
#include<QtCore>
#include <QDialog>
#include <QHBoxLayout>
#include <QListView>
#include <QTableView>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/ostreamwrapper.h>

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
                void generateData(rapidjson::Writer<rapidjson::OStreamWrapper>& writer);

                /**
                * \brief Read and update buttons from data
                */
                void readData(rapidjson::Document& document);

                /**
                * \brief Clear all data in the customize toolbar widget
                */
                void clearContents();

                /**
                * \brief Add customize parent tab
                * \param string tab name
                */
                CustomizeParentTab* addParentTabManual(std::string tabName);

            signals:
                /**
                * \brief Signal for customize widget being closed
                */
                void customizeWidgetClosed();
                
                /**
                * \brief Signal for default settings being clicked
                */
                void defaultSettingsLoad();

            public slots:
                /**
                * \brief Slot for add parent tab button in the widget
                */
                void addParentTab();

                /**
                * \brief Close button for parent tab clicked which removes the tab
                */
                void parentTabClosed(int index);

                /**
                * \brief Load toolbar file
                */
                void loadToolbarFile();

                /**
                * \brief Default button clicked
                */
                void defaultButtonClicked();

                /**
                * \brief Cancel button clicked
                */
                void cancelButtonClicked();

                /**
                * \brief ok button clicked
                */
                void okButtonClicked();

            private:
                void initializeGroupList();

                void initializeParentTab();

                void reAddButtons();

                void addPlusButton();

            private:
                Ui::CustomizeToolbar* ui;
                Toolbar* _toolbar;
                bool _saveOnClose;
            };
        }
    }
}
