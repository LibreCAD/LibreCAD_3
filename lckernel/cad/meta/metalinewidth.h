#pragma once

#include <string>

#include "cad/const.h"

#include "cad/interface/metatype.h"
#include "cad/const.h"
#include <iostream>


namespace lc {

    class MetaLineWidthByBlock : public EntityMetaType {
        public:
            MetaLineWidthByBlock();
            virtual ~MetaLineWidthByBlock() = default;

            virtual const std::string metaTypeID() const;

            virtual const std::string id () const {
                return MetaLineWidthByBlock::LCMETANAME();
            }

            static const std::string LCMETANAME() {
                return "_LINEWIDTH";
            }
        private:
            friend std::ostream& operator<<(std::ostream& os, const MetaLineWidthByBlock& lineWidth) {
                os << "MetaLineWidthByBlock()";
                return os;
            }
    };

    class MetaLineWidthByValue : public EntityMetaType, public DocumentMetaType {
        public:

            MetaLineWidthByValue();
            MetaLineWidthByValue(const double width);
            virtual ~MetaLineWidthByValue() = default;
            double width() const;

            virtual const std::string metaTypeID() const;

            virtual const std::string id () const {
                return MetaLineWidthByValue::LCMETANAME() + "_" + std::to_string(_width);
            }

            virtual const std::string name() const override {
                return MetaLineWidthByValue::LCMETANAME();
            }

            static const std::string LCMETANAME() {
                return "_LINEWIDTH";
            }
        private:
            double _width;

        private:
            friend std::ostream& operator<<(std::ostream& os, const MetaLineWidthByValue& lineWidth) {
                os << "MetaLineWidthByValue(width=" <<  lineWidth.width() << ")";
                return os;
            }
    };

    using MetaLineWidthByValue_SPtr = std::shared_ptr<MetaLineWidthByValue>;
    using MetaLineWidthByValue_CSPtr = std::shared_ptr<const MetaLineWidthByValue>;
    using MetaLineWidthByBlock_SPtr = std::shared_ptr<MetaLineWidthByBlock>;
    using MetaLineWidthByBlock_CSPtr = std::shared_ptr<const MetaLineWidthByBlock>;
}

// LINEWIDTH_H
