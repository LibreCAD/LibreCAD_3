#pragma once

#include "cad/const.h"

#include "color.h"
#include "linewidth.h"
#include "cad/interface/variantable.h"
#include "cad/interface/metatype.h"
#include "cad/const.h"
namespace lc {

    /*!
     * \deprecated Pen is deperecated a user must use color or linewdith
     */
    class Pen : MetaType {
        public:

            Pen();
            Pen(Color* color, LineWidth* lineWidth);
            virtual ~Pen();

            LineWidth* lineWidth() const;
            Color* color() const;

            bool variantValid() const {
                return _color != nullptr && _lineWidth != nullptr;
            }

            metaTypeId metaName() const {
                return MetaType::PEN;
            }

        private:

            Color* _color;
            LineWidth* _lineWidth;
    };
}


// PEN_H
