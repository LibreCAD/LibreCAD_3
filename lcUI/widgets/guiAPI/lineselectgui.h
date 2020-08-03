#pragma once

#include "inputgui.h"
#include <widgets/linewidthselect.h>
#include <widgets/linepatternselect.h>
#include <widgets/colorselect.h>

namespace lc {
    namespace ui {
        namespace api {
            class LineSelectGUI : public InputGUI
            {
            public:
                LineSelectGUI(CadMdiChild* mdiChild, std::string label, QWidget* parent = nullptr);

                void getLuaValue(kaguya::LuaRef& table) override;

                void hideLabel() override;

            protected:
                void copyValue(QDataStream& stream) override;

                void pasteValue(QDataStream& stream) override;

            private:
                lc::ui::widgets::LinePatternSelect _linePatternSelect;
                lc::ui::widgets::LineWidthSelect _lineWidthSelect;
                lc::ui::widgets::ColorSelect _colorSelect;
            };
        }
    }
}
