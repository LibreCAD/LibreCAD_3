#pragma once

#include <cad/meta/dxflinepattern.h>
#include <cad/meta/metacolor.h>
#include <cad/meta/metalinewidth.h>
#include <cad/base/metainfo.h>

namespace lc {
    namespace ui {
        namespace widgets {
            class LinePatternSelect;
            class LineWidthSelect;
            class ColorSelect;
        }

        class MetaInfoManager {
            friend class widgets::LinePatternSelect;
            friend class widgets::LineWidthSelect;
            friend class widgets::ColorSelect;

            public:
                MetaInfoManager();

                /**
                 * @brief Generate new pointer to MetaInfo
                 * @return MetaInfo_CSPtr or nullptr if no properties are available
                 */
                meta::MetaInfo_CSPtr metaInfo();

                meta::DxfLinePattern_CSPtr linePattern() const;
                meta::MetaColor_CSPtr color() const;
                meta::MetaLineWidth_CSPtr lineWidth() const;

            private:
                /**
                 * @brief Generate a new MetaInfo if needed and add the meta type if it's valid
                 * @param metaInfo Actual MetaInfo (or nullptr)
                 * @param metaType MetaType to add (or nullptr)
                 * @return MetaInfo, needed if a new MetaInfo was generated
                 */
                meta::MetaInfo_SPtr addMetaType(meta::MetaInfo_SPtr metaInfo, meta::EntityMetaType_CSPtr metaType);

                void setLinePattern(const meta::DxfLinePattern_CSPtr& linePattern);
                void setColor(const meta::MetaColor_CSPtr& color);
                void setLineWidth(const meta::MetaLineWidth_CSPtr& lineWidth);

                meta::DxfLinePattern_CSPtr _linePattern;
                meta::MetaColor_CSPtr _color;
                meta::MetaLineWidth_CSPtr _lineWidth;
        };

        DECLARE_SHORT_SHARED_PTR(MetaInfoManager)
    }
}