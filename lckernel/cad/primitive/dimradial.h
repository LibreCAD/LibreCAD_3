#pragma once

#include "cad/const.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"

#include "cad/primitive/dimension.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/math/lcmath.h"
#include <cad/primitive/point.h>

namespace lc {
    class DimRadial;
    typedef std::shared_ptr<DimRadial> DimRadial_SPtr;
    typedef std::shared_ptr<const DimRadial> DimRadial_CSPtr;

    /**
    * DimRadial dimension
    * DefinitionPoint hold's the center of the radius
    * DefinitionPoint2 holds the circle/arc edge
    * CenterText holds' teh location of the text
    */
    class DimRadial : public std::enable_shared_from_this<DimRadial>, public CADEntity, public Dimension {
        public:

            /**
                 * @brief DimRadial
                 * @param Dimension dimension
                 * @param double leader
                 * @param Layer_CSPtr layer
                 */
            DimRadial(geo::Coordinate const& definitionPoint, geo::Coordinate const& middleOfText, TextConst::AttachmentPoint const& attachmentPoint, double angle, double const lineSpacingFactor,
                      TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                      geo::Coordinate const& definitionPoint2,
                      const double leader, const Layer_CSPtr layer);

            /**
             * @brief DimRadial
             * @param Dimension dimension
             * @param double leader
             * @param Layer_CSPtr layer
             * @param MetaTypes_CSPtr metaTypes
             */
            DimRadial(geo::Coordinate const& definitionPoint, geo::Coordinate const& middleOfText, TextConst::AttachmentPoint const& attachmentPoint, double angle, double const lineSpacingFactor,
                      TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                      geo::Coordinate const& definitionPoint2,
                      const double leader, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);
            /**
             * Simplified version that set's the midpoint to the middle of the radius and angle to the angle of  definitionPoint and definitionPoint2
            */
            DimRadial(geo::Coordinate const& definitionPoint, TextConst::AttachmentPoint const& attachmentPoint, double const lineSpacingFactor,
                      TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,  geo::Coordinate const& definitionPoint2,
                      const double leader, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

            DimRadial(const DimRadial_CSPtr other, bool sameID = false);
        public:
            /**
             * @brief move, moves by an offset
             * @param geo::Coordinate offset
             * @return CADEntity_CSPtr moved entity
             */
            virtual CADEntity_CSPtr move(const geo::Coordinate& offset) const;

            /**
             * @brief copy, copies by an offset
             * @param geo::Coordinate offset
             * @return CADEntity_CSPtr copied entity
             */
            virtual CADEntity_CSPtr copy(const geo::Coordinate& offset) const;

            /**
             * @brief rotate, rotate operation
             * @param geo::Coordinate rotation_center
             * @param double rotation_angle
             * @return CADEntity_CSPtr rotated entity
             */
            virtual CADEntity_CSPtr rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const;

            /**
             * @brief scale, scales the entity
             * @param geo::Coordinate scale_center
             * @param double scale_factor
             * @return
             */
            virtual CADEntity_CSPtr scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const;

            /**
             * @brief boundingBox of the entity
             * @return geo::Area area
             */
            virtual const geo::Area boundingBox() const;

            virtual CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const;


            double leader() const;

            geo::Coordinate definitionPoint2() const; // Where definitionPoint is the center, definitionPoint2 specifies the 'edge'

        protected:
            const double _leader;
            const geo::Coordinate _definitionPoint2;
        public:
            virtual void accept(const geo::Vector& o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(Point_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(Line_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(Circle_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(Arc_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(Ellipse_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(Text_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(Spline_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(MText_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(DimAligned_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(DimAngular_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(DimDiametric_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(DimLinear_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(DimRadial_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(CADEntity_CSPtr o, EntityVisitor& ei) const {
                o->accept(shared_from_this(), ei);
            }
            virtual void dispatch(EntityDispatch& ed) const {
                ed.visit(shared_from_this());
            }
    };
}
