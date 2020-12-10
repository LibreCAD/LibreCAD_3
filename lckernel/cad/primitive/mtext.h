#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"
#include "textbase.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/math/lcmath.h"
#include "cad/primitive/textconst.h"
#include <cad/primitive/point.h>
#include "cad/interface/draggable.h"
#include <cad/builders/mtext.h>

namespace lc {
    namespace entity {
        class MText : public std::enable_shared_from_this<MText>, public CADEntity, public TextBase, virtual public Visitable, public Draggable {
            friend class builder::MTextBuilder;

        public:
            /**
            * @brief MText, default constructor
            * @param geo::Coordinate insertion_point, insertion_point of the text
            * @param double height, height of the text
            * @param string text_value, the text itself
            * @param double angle, angle of obliqueness
            * @param string style, name of text style
            * @param MTextConst::DrawingDirection textgeneration, MText drawing direction
            * @param MTextConst::HAlign halign, Horizontal alignment
            * @param MTextConst::VAlign valign, Vertical alignment
            * @param Layer_CSPtr layer, Layer of the entity
            * @param metatypes metatypes of the cad entity
            */
            MText(geo::Coordinate insertion_point,
                std::string text_value,
                double height,
                double angle,
                std::string style,
                const TextConst::DrawingDirection textgeneration,
                const TextConst::HAlign halign,
                const TextConst::VAlign valign,
                bool underlined,
                bool strikethrough,
                bool bold,
                bool italic,
                meta::Layer_CSPtr layer,
                meta::MetaInfo_CSPtr metaInfo = nullptr,
                meta::Block_CSPtr block = nullptr
            );

            MText(const MText_CSPtr& other, bool sameID);

        private:
            MText(const builder::MTextBuilder& builder);

        public:
            const bool _underlined;
            const bool _strikethrough;
            const bool _bold;
            const bool _italic;

            bool underlined() const {
                return _underlined;
            }

            bool strikethrough() const {
                return _strikethrough;
            }

            bool bold() const {
                return _bold;
            }

            bool italic() const {
                return _italic;
            }

            /**
            * @brief move, moves by an offset
            * @param geo::Coordinate offset
            * @return CADEntity_CSPtr moved entity
            */
        public:
            // Methods from CADEntity
            CADEntity_CSPtr move(const geo::Coordinate& offset) const override;
            CADEntity_CSPtr copy(const geo::Coordinate& offset) const override;
            CADEntity_CSPtr rotate(const geo::Coordinate& rotation_center, double rotation_angle) const override;
            CADEntity_CSPtr scale(const geo::Coordinate& scale_center,
                const geo::Coordinate& scale_factor) const override;
            CADEntity_CSPtr mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const override {
                return NULL; /// @todo no return statement, not implemented in lwpolyline.cpp
            }
            const geo::Area boundingBox() const override;
            CADEntity_CSPtr modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const override;
            void accept(GeoEntityVisitor& v) const override {
                v.visit(*this);
            }
            void dispatch(EntityDispatch& ed) const override {
                ed.visit(shared_from_this());
            }
            std::map<unsigned int, lc::geo::Coordinate> dragPoints() const override;
            CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;

            PropertiesMap availableProperties() const override;

            CADEntity_CSPtr setProperties(const PropertiesMap& propertiesMap) const override;
        };

        DECLARE_SHORT_SHARED_PTR(MText)
    }
}

