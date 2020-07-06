#pragma once

#include <QWidget>
#include <QTabWidget>

#include <string>

#include <widgets/guiAPI/toolbartab.h>
#include <widgets/customizeToolbar/customizegrouptab.h>

namespace lc {
    namespace ui {
        namespace widgets {
            /**
            * \brief Represents a tab of the toolbar in the widget
            */
            class CustomizeParentTab : public QTabWidget
            {
                Q_OBJECT

            public:
                /**
                * \brief CustomizeToolbar constructor
                * \param pointer to ToolbarTab toolbartab
                * \param pointer to qwidget parent
                */
                CustomizeParentTab(lc::ui::api::ToolbarTab* toolbarTab, QWidget* parent = nullptr);

                /**
                * \brief CustomizeToolbar constructor
                * \param QString label of group
                * \param pointer to qwidget parent
                */
                CustomizeParentTab(QString label , QWidget* parent = nullptr);

                /**
                * \brief get label of toolbar group
                */
                std::string label() const;

                /**
                * \brief remove all group tabs from parenttab
                */
                void clearContents();

                /**
                * \brief Add customize group tab
                * \param string group name
                * \param pointer to group tab
                */
                CustomizeGroupTab* addGroupTabManual(std::string groupName, int width);

            public slots:
                /**
                * \brief Slot for add group button
                */
                void addGroupTab();

                /**
                * \brief Close button for group tab which removes group
                */
                void groupTabClosed(int index);

            private:
                void init();

            private:
                std::string _label;
            };
        }
    }
}
