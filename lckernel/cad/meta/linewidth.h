#pragma once

#include <string>

#include "cad/const.h"

#include "cad/interface/metatype.h"
#include "cad/const.h"

namespace lc {

    class LineWidth : public MetaType {
        public:


            LineWidth();
            LineWidth(const double width);
            virtual ~LineWidth() {}
            double width() const;

            metaTypeId metaName() const {
                return MetaType::LINEWIDTH;
            }

        private:
            double _width;
    };


}

// LINEWIDTH_H
