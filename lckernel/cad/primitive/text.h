#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"


#include "cad/geometry/geocoordinate.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/math/lcmath.h"
#include "cad/primitive/textconst.h"
#include <cad/primitive/point.h>
#include "cad/interface/draggable.h"

namespace lc {
    namespace entity {
        class Text : public std::enable_shared_from_this<Text>, public CADEntity, virtual public Visitable, public Draggable {
        public:
            /**
            * @brief Text, default constructor
            * @param geo::Coordinate insertion_point, insertion_point of the text
            * @param double height, height of the text
            * @param string text_value, the text itself
            * @param double angle, angle of obliqueness
            * @param string style, name of text style
            * @param TextConst::DrawingDirection textgeneration, Text drawing direction
            * @param TextConst::HAlign halign, Horizontal alignment
            * @param TextConst::VAlign valign, Vertical alignment
            * @param Layer_CSPtr layer, Layer of the entity
            * @param metatypes metatypes of the cad entity
            */
            Text(geo::Coordinate insertion_point,
                 std::string text_value,
                 double height,
                 double angle,
                 std::string style,
                 const TextConst::DrawingDirection textgeneration,
                 const TextConst::HAlign halign,
                 const TextConst::VAlign valign,
                 Layer_CSPtr layer,
                 MetaInfo_CSPtr metaInfo = nullptr,
                 Block_CSPtr block = nullptr
            );

            Text(const Text_CSPtr &other, bool sameID);

            const geo::Coordinate _insertion_point;
            const std::string _text_value;
            const double _height;
            const double _angle;
            const std::string _style;
            const TextConst::DrawingDirection _textgeneration;
            const TextConst::VAlign _valign;
            const TextConst::HAlign _halign;

            geo::Coordinate insertion_point() const {
                return _insertion_point;
            }

            std::string const &text_value() const {
                return _text_value;
            }

            double const angle() const {
                return _angle;
            }

            std::string const &style() const {
                return _style;
            }

            TextConst::DrawingDirection const &textgeneration() const {
                return _textgeneration;
            }

            TextConst::VAlign const &valign() const {
                return _valign;
            }

            TextConst::HAlign const &halign() const {
                return _halign;
            }

            double height() const {
                return _height;
            }

            /**
            * @brief move, moves by an offset
            * @param geo::Coordinate offset
            * @return CADEntity_CSPtr moved entity
            */
            CADEntity_CSPtr move(const geo::Coordinate &offset) const override;

            /**
            * @brief copy, copies line by an offset
            * @param geo::Coordinate offset
            * @return CADEntity_CSPtr copied entity
            */
            CADEntity_CSPtr copy(const geo::Coordinate &offset) const override;

            /**
            * @brief rotate, rotate operation
            * @param geo::Coordinate rotation_center
            * @param double rotation_angle
            * @return CADEntity_CSPtr rotated entity
            */
            CADEntity_CSPtr rotate(const geo::Coordinate &rotation_center, double rotation_angle) const override;

            /**
            * @brief scale, scales the entity
            * @param geo::Coordinate scale_center
            * @param double scale_factor
            * @return
            */
            CADEntity_CSPtr scale(const geo::Coordinate &scale_center,
                                  const geo::Coordinate &scale_factor) const override;

            CADEntity_CSPtr mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const override {
                return NULL; // TODO: no return statement, not implemented in lwpolyline.cpp
            }

            /**
            * @brief boundingBox of the entity
            * @return geo::Area area
            */
            const geo::Area boundingBox() const override;

            CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo, Block_CSPtr block) const override;

        public:
            void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            void dispatch(EntityDispatch &ed) const override {
                ed.visit(shared_from_this());
            }

        public:
            std::map<unsigned int, lc::geo::Coordinate> dragPoints() const override;
            CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;
        };

        DECLARE_SHORT_SHARED_PTR(Text)
    }
}

