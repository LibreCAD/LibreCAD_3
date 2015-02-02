#pragma once

#include "id.h"
#include "cad/const.h"
#include "metainfo.h"
#include "cad/interface/metatype.h"
#include "cad/meta/layer.h"
#include "cad/geometry/geoarea.h"

namespace lc {
    namespace geo {
        class Vector;
        class Coordinate;
    }
    class EntityVisitor;
    class EntityDispatch;

    class Arc;
    typedef std::shared_ptr<Arc> Arc_SPtr;
    typedef std::shared_ptr<const Arc> Arc_CSPtr;

    class Point;
    typedef std::shared_ptr<Point> Point_SPtr;
    typedef std::shared_ptr<const Point> Point_CSPtr;

    class Line;
    typedef std::shared_ptr<Line> Line_SPtr;
    typedef std::shared_ptr<const Line> Line_CSPtr;

    class Text;
    typedef std::shared_ptr<Text> Text_SPtr;
    typedef std::shared_ptr<const Text> Text_CSPtr;

    class Spline;
    typedef std::shared_ptr<Spline> Spline_SPtr;
    typedef std::shared_ptr<const Spline> Spline_CSPtr;

    class Ellipse;
    typedef std::shared_ptr<Ellipse> Ellipse_SPtr;
    typedef std::shared_ptr<const Ellipse> Ellipse_CSPtr;

    class Circle;
    typedef std::shared_ptr<Circle> Circle_SPtr;
    typedef std::shared_ptr<const Circle> Circle_CSPtr;

    class CADEntity;
    typedef std::shared_ptr<CADEntity> CADEntity_SPtr;
    typedef std::shared_ptr<const CADEntity> CADEntity_CSPtr;

    class MText;
    typedef std::shared_ptr<MText> MText_SPtr;
    typedef std::shared_ptr<const MText> MText_CSPtr;

    class DimAligned;
    typedef std::shared_ptr<DimAligned> DimAligned_SPtr;
    typedef std::shared_ptr<const DimAligned> DimAligned_CSPtr;

    class DimAngular;
    typedef std::shared_ptr<DimAngular> DimAngular_SPtr;
    typedef std::shared_ptr<const DimAngular> DimAngular_CSPtr;

    class DimDiametric;
    typedef std::shared_ptr<DimDiametric> DimDiametric_SPtr;
    typedef std::shared_ptr<const DimDiametric> DimDiametric_CSPtr;

    class DimLinear;
    typedef std::shared_ptr<DimLinear> DimLinear_SPtr;
    typedef std::shared_ptr<const DimLinear> DimLinear_CSPtr;

    class DimRadial;
    typedef std::shared_ptr<DimRadial> DimRadial_SPtr;
    typedef std::shared_ptr<const DimRadial> DimRadial_CSPtr;

    class Quadratic;
    typedef std::shared_ptr<Quadratic> Quadratic_SPtr;
    typedef std::shared_ptr<const Quadratic> Quadratic_CSPtr;

    class CADEntity;
    typedef std::shared_ptr<CADEntity> CADEntity_SPtr;
    typedef std::shared_ptr<const CADEntity> CADEntity_CSPtr;

    /**
     *Class that all CAD entities must inherit
     *
     */
    class  CADEntity  : public ID {
        public:
            CADEntity() {
            }

            /*!
                         * \brief Default CADEntity Constructor.
                         * \sa lc::ID
                         * \sa lc::MetaInfo
                         */
            CADEntity(Layer_CSPtr _layer);
            /*!
             * \brief CADEntity Constructor
             *
             * \param metaTypes A list of metatypes associated with this line
             * \sa lc::Color
             * \sa lc::LineWidth
             * \sa lc::MetaType
             */
            CADEntity(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);
            CADEntity(CADEntity_CSPtr cadEntity, bool sameID);
            CADEntity(CADEntity_CSPtr cadEntity);


            virtual void accept(const geo::Vector&, EntityVisitor&) const = 0;

            virtual void accept(Line_CSPtr, EntityVisitor&) const = 0;
            virtual void accept(Point_CSPtr, EntityVisitor&) const = 0;
            virtual void accept(Circle_CSPtr, EntityVisitor&) const = 0;
            virtual void accept(Arc_CSPtr, EntityVisitor&) const = 0;
            virtual void accept(Ellipse_CSPtr, EntityVisitor&) const = 0;
            virtual void accept(Text_CSPtr, EntityVisitor&) const = 0;
            virtual void accept(MText_CSPtr, EntityVisitor&) const = 0;

            virtual void accept(DimAligned_CSPtr, EntityVisitor&) const = 0;
            virtual void accept(DimAngular_CSPtr, EntityVisitor&) const = 0;
            virtual void accept(DimDiametric_CSPtr, EntityVisitor&) const = 0;
            virtual void accept(DimLinear_CSPtr, EntityVisitor&) const = 0;
            virtual void accept(DimRadial_CSPtr, EntityVisitor&) const = 0;

            virtual void accept(Spline_CSPtr, EntityVisitor&) const = 0;

            virtual void accept(CADEntity_CSPtr, EntityVisitor&) const = 0;

            virtual void dispatch(EntityDispatch&) const = 0;
            /*!
             * \brief Function for Move
             *
             * \param offset the offset by which entity is to be moved
             * \return CADEntity std::shared_ptr
             */
            virtual CADEntity_CSPtr move(const geo::Coordinate& offset) const = 0;

            /*!
             * \brief Function for Copy
             *
             * \param offset the offset by which entity is to be copied
             * \return CADEntity std::shared_ptr
             */
            virtual CADEntity_CSPtr copy(const geo::Coordinate& offset) const = 0;
            /*!
             * \brief Function implementation for rotate.
             *
             * \param angle angle by which the entity is to be rotated.
             * \return CADEntity std::shared_ptr
             */
            virtual CADEntity_CSPtr rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const = 0;
            /*!
             * \brief Function implementation for Scale.
             *
             * \param scale_center
             * \param scale_factor
             * \return CADEntity std::shared_ptr
             */
            virtual CADEntity_CSPtr scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const = 0;

            /**
             * @brief boundingBox
             * Return the bounding box of this entity.
             * This should be as tight around the entity as possible but care should be taken that this routine
             * is also as fast as possible. in essense, favor speed over tightness.
             * This routine will in generally be used during testing if entities cross for example to find snap points
             * and to build relationships of tree for example using quad trees
             * @return geo::Area
             */
            virtual const geo::Area boundingBox() const = 0;

            /**
            * @brief modify
            * Return a new entity with the same ID bit with possible modified metainfo and/pr layer information
            * #return new entity with same ID
            */
            virtual CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const = 0;

            /*!
             * \brief layer
             * return the layer this entity is placed on
             * \return Layer_CSPtr
             */
            Layer_CSPtr layer() const;

            /**
            * Retreive meta information back from this entity
            * returns nullptr when the specific meta info wasn't found
            * example: auto metaData = myEntity.metaInfo<lc::MetaColor>(lc::MetaInfo::_COLOR);
            */
            template <typename T>
            const std::shared_ptr<const T> metaInfo(std::string metaName) const {
                if (_metaInfo && (_metaInfo->find(metaName) != _metaInfo->end())) {
                    return std::dynamic_pointer_cast<const T>(_metaInfo->at(metaName));
                }

                return nullptr;
            }

            MetaInfo_CSPtr metaInfo() const {
                return _metaInfo;
            }

        private:
            Layer_CSPtr _layer;
            MetaInfo_CSPtr _metaInfo;
    };
}
