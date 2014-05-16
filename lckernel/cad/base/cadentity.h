#ifndef CADENTITY_H
#define CADENTITY_H

#include "cad/const.h"
#include "id.h"
#include "metainfo.h"
#include "cadentity.h"
#include "cad/interface/metatype.h"
#include "cad/geometry/geoarea.h"
#include "cad/meta/layer.h"

namespace lc {

    class EntityVisitor;
    class EntityDispatch;

    class Arc;
    typedef std::shared_ptr<Arc> Arc_SPtr;
    typedef std::shared_ptr<const Arc> Arc_CSPtr;

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

    /**
     *Class that all CAD entities must inherit
     *
     */
    class  CADEntity  : public ID, public MetaInfo {
        public:
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
            CADEntity(Layer_CSPtr _layer, std::list<MetaType_CSPtr> metaTypes);

            /*!
             * \brief CADEntity Constructor
             *
             * \param metaTypes A list of metatypes associated with this line
             * \sa lc::Color
             * \sa lc::LineWidth
             * \sa lc::MetaType
             */
            CADEntity(Layer_CSPtr _layer, std::map<MetaType::metaTypeId, MetaType_CSPtr > metaTypes);

            virtual void accept(Line_CSPtr, EntityVisitor&) const = 0;
            virtual void accept(Circle_CSPtr, EntityVisitor&) const = 0;
            virtual void accept(Arc_CSPtr, EntityVisitor&) const = 0;
            virtual void accept(Ellipse_CSPtr, EntityVisitor&) const = 0;
            virtual void accept(Text_CSPtr, EntityVisitor&) const = 0;
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

            /*!
             * \brief layer
             * return the layer this entity is placed on
             * \return
             */
            virtual Layer_CSPtr layer() const;
        private:
            Layer_CSPtr _layer;

    };
}

#endif // CADENTITY_H
