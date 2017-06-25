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
            Block(std::string name, geo::Coordinate base);

            const std::string id() const override;
            const std::string name() const override;

            const geo::Coordinate& base() const;

        private:
            std::string _name;
            geo::Coordinate _base;
    };

    DECLARE_SHORT_SHARED_PTR(Block)
}