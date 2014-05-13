#ifndef CADENTITY_H
#define CADENTITY_H
#include <QList>

#include "cad/const.h"
#include "id.h"
#include "metainfo.h"
#include "cad/interface/metatype.h"
#include "cad/geometry/geoarea.h"

namespace lc {

    class Circle;
    class Line;
    class Arc;
    class Ellipse;
    class Text;
    class Spline;
    class Layer;

    class EntityVisitor;
    class EntityDispatch;

    /**
     *Class that all CAD entities must inherit
     *
     */
    class CADEntity : public ID, public MetaInfo {
        public:
            /*!
             * \brief Default CADEntity Constructor.
             * \sa lc::ID
             * \sa lc::MetaInfo
             */
            CADEntity(std::shared_ptr<const Layer> _layer);
            /*!
             * \brief CADEntity Constructor
             *
             * \param metaTypes A list of metatypes associated with this line
             * \sa lc::Color
             * \sa lc::LineWidth
             * \sa lc::MetaType
             */
            CADEntity(std::shared_ptr<const Layer> _layer, QList<std::shared_ptr<const MetaType> > metaTypes);

            /*!
             * \brief CADEntity Constructor
             *
             * \param metaTypes A list of metatypes associated with this line
             * \sa lc::Color
             * \sa lc::LineWidth
             * \sa lc::MetaType
             */
            CADEntity(std::shared_ptr<const Layer> _layer, QHash<int, std::shared_ptr<const MetaType> > metaTypes);

            virtual void accept(std::shared_ptr<const Line>, EntityVisitor&) const = 0;
            virtual void accept(std::shared_ptr<const Circle>, EntityVisitor&) const = 0;
            virtual void accept(std::shared_ptr<const Arc>, EntityVisitor&) const = 0;
            virtual void accept(std::shared_ptr<const Ellipse>, EntityVisitor&) const = 0;
            virtual void accept(std::shared_ptr<const Text>, EntityVisitor&) const = 0;
            virtual void accept(std::shared_ptr<const CADEntity>, EntityVisitor&) const = 0;
            virtual void dispatch(EntityDispatch&) const = 0;
            /*!
             * \brief Function for Move
             *
             * \param offset the offset by which entity is to be moved
             * \return CADEntity std::shared_ptr
             */
            virtual std::shared_ptr<const CADEntity> move(const geo::Coordinate& offset) const = 0;

            /*!
             * \brief Function for Copy
             *
             * \param offset the offset by which entity is to be copied
             * \return CADEntity std::shared_ptr
             */
            virtual std::shared_ptr<const CADEntity> copy(const geo::Coordinate& offset) const = 0;
            /*!
             * \brief Function implementation for rotate.
             *
             * \param angle angle by which the entity is to be rotated.
             * \return CADEntity std::shared_ptr
             */
            virtual std::shared_ptr<const CADEntity> rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const = 0;
            /*!
             * \brief Function implementation for Scale.
             *
             * \param scale_center
             * \param scale_factor
             * \return CADEntity std::shared_ptr
             */
            virtual std::shared_ptr<const CADEntity> scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const = 0;

            /*!
             * \brief layer
             * return the layer this entity is placed on
             * \return
             */
            virtual std::shared_ptr<const Layer> layer() const;
        private:
            std::shared_ptr<const Layer> _layer;

    };
}

#endif // CADENTITY_H
