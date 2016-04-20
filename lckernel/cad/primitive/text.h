#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"


#include "cad/geometry/geocoordinate.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/math/lcmath.h"
#include "cad/primitive/textconst.h"
#include <cad/primitive/point.h>

namespace lc {
    namespace entity {
        class Text : public std::enable_shared_from_this<Text>, public CADEntity, virtual public Visitable {
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
            */
            Text(const geo::Coordinate &insertion_point,
                 const std::string text_value,
                 const double height,
                 const double angle,
                 const std::string style,
                 const TextConst::DrawingDirection textgeneration,
                 const TextConst::HAlign halign,
                 const TextConst::VAlign valign,
                 const Layer_CSPtr layer);

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
            Text(const geo::Coordinate &insertion_point,
                 const std::string text_value,
                 const double height,
                 const double angle,
                 const std::string style,
                 const TextConst::DrawingDirection textgeneration,
                 const TextConst::HAlign halign,
                 const TextConst::VAlign valign,
                 const Layer_CSPtr layer,
                 const MetaInfo_CSPtr metaInfo);

            Text(const Text_CSPtr &other, bool sameID);

        public:
            const geo::Coordinate _insertion_point;
            const std::string _text_value;
            const double _height;
            const double _angle;
            const std::string _style;
            const TextConst::DrawingDirection _textgeneration;
            const TextConst::VAlign _valign;
            const TextConst::HAlign _halign;
        public:

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

        public:
            /**
            * @brief move, moves by an offset
            * @param geo::Coordinate offset
            * @return CADEntity_CSPtr moved entity
            */
            virtual CADEntity_CSPtr move(const geo::Coordinate &offset) const override;

            /**
            * @brief copy, copies line by an offset
            * @param geo::Coordinate offset
            * @return CADEntity_CSPtr copied entity
            */
            virtual CADEntity_CSPtr copy(const geo::Coordinate &offset) const override;

            /**
            * @brief rotate, rotate operation
            * @param geo::Coordinate rotation_center
            * @param double rotation_angle
            * @return CADEntity_CSPtr rotated entity
            */
            virtual CADEntity_CSPtr rotate(const geo::Coordinate &rotation_center, const double rotation_angle) const override;

            /**
            * @brief scale, scales the entity
            * @param geo::Coordinate scale_center
            * @param double scale_factor
            * @return
            */
            virtual CADEntity_CSPtr scale(const geo::Coordinate &scale_center,
                                          const geo::Coordinate &scale_factor) const override;

            /**
            * @brief boundingBox of the entity
            * @return geo::Area area
            */
            virtual const geo::Area boundingBox() const override;

            virtual CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const override;

        public:
            virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            virtual void dispatch(EntityDispatch &ed) const override {
                ed.visit(shared_from_this());
            }
        };
        using Text_SPtr = std::shared_ptr<Text>;
        using Text_CSPtr = std::shared_ptr<const Text>;
    }
}

