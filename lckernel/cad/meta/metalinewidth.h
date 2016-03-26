#pragma once

#include <string>

#include "cad/const.h"

#include "cad/interface/metatype.h"
#include "cad/const.h"
#include <iostream>


namespace lc {

    class MetaLineWidth : public MetaType {
        public:


            MetaLineWidth();
            MetaLineWidth(const double width);
            virtual ~MetaLineWidth() {}
            double width() const;

            virtual std::string lcMetaName() const;
            static std::string LCMETANAME();

        private:
            double _width;

        private:
            friend std::ostream& operator<<(std::ostream& os, const MetaLineWidth& lineWidth) {
                os << "MetaLineWidth(width=" <<  lineWidth.width() << ")";
                return os;
            }
    };

    using MetaLineWidth_SPtr = std::shared_ptr<MetaLineWidth>;
    using MetaLineWidth_CSPtr = std::shared_ptr<const MetaLineWidth>;
}

// LINEWIDTH_H
