#pragma once

#include <QObject>
#include <QPushButton>

#include <kaguya/kaguya.hpp>

namespace lc {
    namespace ui {
        namespace api {
            class ButtonGUI : public QPushButton
            {
                Q_OBJECT

            public:
                ButtonGUI(std::string label, QWidget* parent = nullptr);

            private:
                std::string _label;
                std::vector<kaguya::LuaRef> _callbacks;
            };
        }
    }
}
