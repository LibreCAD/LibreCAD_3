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
                LineSelectGUI(CadMdiChild* mdiChild, MetaInfoManager_SPtr metaInfoManager, std::string label, QWidget* parent = nullptr);

                void getLuaValue(kaguya::LuaRef& table) override;

                void hideLabel() override;

                void addCallback(kaguya::LuaRef cb);

                void setEntityMetaInfo(lc::entity::CADEntity_CSPtr entity);

            public slots:
                void metaInfoManagerChanged();

            protected:
                void copyValue(QDataStream& stream) override;

                void pasteValue(QDataStream& stream) override;

            private:
                lc::ui::widgets::LinePatternSelect _linePatternSelect;
                lc::ui::widgets::LineWidthSelect _lineWidthSelect;
                lc::ui::widgets::ColorSelect _colorSelect;
                std::vector<kaguya::LuaRef> _callbacks;
            };
        }
    }
}
