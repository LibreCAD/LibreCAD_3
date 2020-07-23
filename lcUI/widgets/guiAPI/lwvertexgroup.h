#pragma once

#include "horizontalgroupgui.h"
#include "coordinategui.h"
#include "numbergui.h"
#include <mainwindow.h>

namespace lc
{
    namespace ui
    {
        namespace api
        {
            class LWVertexGroup : public HorizontalGroupGUI
            {
                Q_OBJECT

            public:
                LWVertexGroup(std::string label, QWidget* parent = nullptr);

                void addCallback(kaguya::LuaRef cb);

                void setMainWindow(lc::ui::MainWindow* mainWindowIn);

                void getLuaValue(kaguya::LuaRef& table) override;

                void setKey(const std::string& keyIn) override;

            private:
                lc::ui::api::CoordinateGUI* coordgui;
                lc::ui::api::NumberGUI* startWidth;
                lc::ui::api::NumberGUI* endWidth;
            };
        }
    }
}
