#pragma once

#include <QWidget>
#include <QTabWidget>

#include <widgets/guiAPI/toolbartab.h>

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
