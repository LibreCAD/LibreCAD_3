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
    class CADEntity;

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
            CADEntity();
            /*!
             * \brief CADEntity Constructor
             *
             * \param metaTypes A list of metatypes associated with this line
             * \sa lc::Color
             * \sa lc::LineWidth
             * \sa lc::MetaType
             */
            CADEntity(QList<shared_ptr<const lc::MetaType> > metaTypes);
            virtual void accept(shared_ptr<const lc::Line>, EntityVisitor&) const = 0;
            virtual void accept(shared_ptr<const lc::Circle>, EntityVisitor&) const = 0;
            virtual void accept(shared_ptr<const lc::Arc>, EntityVisitor&) const = 0;
            virtual void accept(shared_ptr<const lc::Ellipse>, EntityVisitor&) const = 0;
            virtual void accept(shared_ptr<const lc::Text>, EntityVisitor&) const = 0;
            virtual void accept(shared_ptr<const lc::CADEntity>, EntityVisitor&) const = 0;
            virtual void dispatch(EntityDispatch&) const = 0;
            /*!
             * \brief Function for Move
             *
             * \param offset the offset by which entity is to be moved
             * \return CADEntity shared_ptr
             */
            virtual shared_ptr<const lc::CADEntity> move(const geo::Coordinate& offset) const = 0;
            /*!
             * \brief Function implementation for rotate.
             *
             * \param angle angle by which the entity is to be rotated.
             * \return CADEntity shared_ptr
             */
            virtual shared_ptr<const lc::CADEntity> rotate(const lc::geo::Coordinate& rotation_point, const double& angle) const = 0;
    };
}

#endif // CADENTITY_H
