#pragma once

#include "inputgui.h"
#include <QWidget>

namespace Ui {
    class EntityGUI; 
}

namespace lc {
    namespace ui {
        namespace api {
            class EntityGUI : public InputGUI
            {
                Q_OBJECT

            public:
                EntityGUI(std::string label, QWidget* parent = nullptr);
                ~EntityGUI();

                void getLuaValue(kaguya::LuaRef& table);

            private:
                Ui::EntityGUI* ui;
            };
        }
    }
}
