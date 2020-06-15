#include "inputgui.h"

namespace lc {
    namespace ui {
        namespace api {
            class ButtonGroupGUI : InputGUI
            {
                Q_OBJECT

            public:
                ButtonGroupGUI(std::string label, QWidget* parent = nullptr);
            };
        }
    }
}
