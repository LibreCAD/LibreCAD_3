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
            * @param double width, reference rectangle width, 0 for none
            * @param MTextConst::MTextDrawingDirection drawingDirection, DXF group 72
            * @param double lineSpacingFactor, multiple of the single-line spacing
            * @param MTextConst::LineSpacingStyle lineSpacingStyle, DXF group 73
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
                double width,
                const TextConst::MTextDrawingDirection drawingDirection,
                double lineSpacingFactor,
                const TextConst::LineSpacingStyle lineSpacingStyle,
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

            /// DXF group 41: the width of the reference rectangle text wraps
            /// inside.  0 means there is none -- the value every MText this
            /// build creates carries, because nothing here wraps yet.
            const double _width;

            /// DXF group 72.  Not TextBase::_textgeneration, which is TEXT's
            /// group-71 mirroring flag and has no MTEXT meaning.
            const TextConst::MTextDrawingDirection _drawingDirection;

            /// DXF group 44: a multiple of the single-line spacing, 0.25 to 4.
            const double _lineSpacingFactor;

            /// DXF group 73.
            const TextConst::LineSpacingStyle _lineSpacingStyle;

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

            double width() const {
                return _width;
            }

            TextConst::MTextDrawingDirection drawingDirection() const {
                return _drawingDirection;
            }

            double lineSpacingFactor() const {
                return _lineSpacingFactor;
            }

            TextConst::LineSpacingStyle lineSpacingStyle() const {
                return _lineSpacingStyle;
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

