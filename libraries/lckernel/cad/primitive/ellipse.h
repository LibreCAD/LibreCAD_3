#ifndef ELLIPSE_H
#define ELLIPSE_H

#include "cad/const.h"
#include "cad/interface/entityinteraction.h"

#include "lckernel_global.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geoellipse.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"

namespace lc {
    /*!
     * \brief A ellipse that can be put in a drawing
     *
     * A lc::Ellipse is a graphics elipse item that can be put on a drawing using a CreateEntity operation.
     *
     * \sa lc::CreateEntities::append
     *
     * \author R. van Twisk
     *
     * \date 2012-04-16
     */
    class Ellipse : public std::tr1::enable_shared_from_this<Ellipse>, public CADEntity, public geo::Ellipse {
        public:
            Ellipse(const geo::Coordinate& center, const geo::Coordinate& majorP, double minorRadius, double startAngle, double endAngle);
            Ellipse(const geo::Coordinate& center, const geo::Coordinate& majorP, double minorRadius, double startAngle, double endAngle, const QList<std::tr1::shared_ptr<const lc::MetaType> >& metaTypes);

        public:
            virtual void accept(std::tr1::shared_ptr<const lc::Line> o, EntityInteraction& ei) const {
                ei.visitInteraction(shared_from_this(), o);
            }
            virtual void accept(std::tr1::shared_ptr<const lc::Circle> o, EntityInteraction& ei) const {
                ei.visitInteraction(shared_from_this(), o);
            }
            virtual void accept(std::tr1::shared_ptr<const lc::Arc> o, EntityInteraction& ei) const {
                ei.visitInteraction(shared_from_this(), o);
            }
            virtual void accept(std::tr1::shared_ptr<const lc::Ellipse> o, EntityInteraction& ei) const {
                ei.visitInteraction(shared_from_this(), o);
            }
            virtual void accept(std::tr1::shared_ptr<const lc::CADEntity> o, EntityInteraction& ei) const {
                o->accept(shared_from_this(), ei);
            }

    };
}

#endif // ELLIPSE_H
