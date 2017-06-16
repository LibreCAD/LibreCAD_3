#pragma once

#include <cad/interface/metatype.h>
#include <vector>
#include <cad/geometry/geocoordinate.h>
#include "metacolor.h"
#include "dxflinepattern.h"
#include "metalinewidth.h"

#define BLOCK_ID_PREFIX "BLOCK"

namespace lc {
    class Block : public DocumentMetaType {
        public:
            Block(std::string name, geo::Coordinate base,
                  lc::MetaColor_CSPtr color, lc::MetaLineWidth_CSPtr lineWidth, lc::DxfLinePattern_CSPtr linePattern);

            const std::string id() const override;
            const std::string name() const override;

            const geo::Coordinate& base() const;
            const MetaColor_CSPtr& color() const;
            const EntityMetaType_CSPtr& lineWidth() const;
            const DxfLinePattern_CSPtr& linePattern() const;

        private:
            std::string _name;
            geo::Coordinate _base;
            lc::MetaColor_CSPtr _color; //TODO: that's should probably be changed to a pointer to lc::Color
            lc::MetaLineWidth_CSPtr _lineWidth;
            lc::DxfLinePattern_CSPtr _linePattern;
    };

    using Block_SPtr = std::shared_ptr<Block>;
    using Block_CSPtr = std::shared_ptr<const Block>;
}