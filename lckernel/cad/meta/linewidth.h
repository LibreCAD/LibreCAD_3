#ifndef LINEWIDTH_H
#define LINEWIDTH_H

#include <string>

#include "cad/const.h"

#include "cad/interface/variantable.h"
#include "cad/interface/metatype.h"
#include "cad/const.h"

namespace lc {

    class LineWidth : public MetaType {
        public:
            enum Method {
                Invalid = -1,
                ByLayer = 0,
                ByEntity = 1
            };


            LineWidth();
            LineWidth(const double width);
            LineWidth(LineWidth::Method method);
            virtual ~LineWidth() {}
            Method method() const;
            double width() const;

            bool variantValid() const {
                return !(_method == LineWidth::Invalid);
            }

            metaTypeId metaName() const {
                return MetaType::LINEWIDTH;
            }

        private:
            Method _method;
            double _width;
    };


}

#endif // LINEWIDTH_H
