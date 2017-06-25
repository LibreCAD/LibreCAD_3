#pragma once

#include <string>

#include "cad/const.h"

#include "cad/interface/metatype.h"
#include "cad/const.h"
#include <iostream>

namespace lc {
    class MetaLineWidth : public EntityMetaType {
        public:
            static const std::string LCMETANAME();

            virtual const std::string id() const override;

        private:
            friend std::ostream& operator<<(std::ostream& os, const MetaLineWidth& lineWidth) {
                os << "MetaLineWidth()";
                return os;
            }
    };

    class MetaLineWidthByBlock : public MetaLineWidth {
        public:
            MetaLineWidthByBlock();
            virtual ~MetaLineWidthByBlock() = default;

            virtual const std::string metaTypeID() const override;

        private:
            friend std::ostream& operator<<(std::ostream& os, const MetaLineWidthByBlock& lineWidth) {
                os << "MetaLineWidthByBlock()";
                return os;
            }
    };

    class MetaLineWidthByValue : public MetaLineWidth, public DocumentMetaType {
        public:

            MetaLineWidthByValue();
            MetaLineWidthByValue(const double width);
            virtual ~MetaLineWidthByValue() = default;
            double width() const;

            virtual const std::string metaTypeID() const override;

            virtual const std::string id () const override {
                return MetaLineWidthByValue::LCMETANAME() + "_" + std::to_string(_width);
            }

            const std::string name() const override;

        private:
            double _width;

        private:
            friend std::ostream& operator<<(std::ostream& os, const MetaLineWidthByValue& lineWidth) {
                os << "MetaLineWidthByValue(width=" <<  lineWidth.width() << ")";
                return os;
            }
    };

    DECLARE_SHORT_SHARED_PTR(MetaLineWidth)
    DECLARE_SHORT_SHARED_PTR(MetaLineWidthByBlock)
    DECLARE_SHORT_SHARED_PTR(MetaLineWidthByValue)
}
