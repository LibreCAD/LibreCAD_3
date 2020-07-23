#pragma once

#include "inputgui.h"
#include <QHBoxLayout>

#include <set>

namespace lc {
    namespace ui {
        namespace api {
            /**
            * \brief Horizontal Group GUI Widget
            */
            class HorizontalGroupGUI : public InputGUI
            {
                Q_OBJECT

            public:
                /**
                * \brief Horizontal Group GUI Constructor
                * \param string group label
                * \param parent qwidget parent
                */
                HorizontalGroupGUI(std::string label, bool vertical = false, QWidget* parent = nullptr);

                /**
                * \brief add button to group
                * \param newWidget pointer to QWidget
                */
                void addWidget(const std::string& key, QWidget* newWidget);

                /**
                * \brief Return lua value
                * \param LuaRef value
                */
                void getLuaValue(kaguya::LuaRef& table) override;

                /**
                * \brief Get keys of all widgets added to the group
                * \param set of string keys
                */
                std::set<std::string> getKeys();

            protected:
                std::vector<QWidget*> _widgets;
                QLayout* qboxlayout;
                std::set<std::string> _addedKeys;
                std::set<std::string> _buttonKeys;
            };
        }
    }
}
