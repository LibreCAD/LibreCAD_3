#ifndef ELLIPSE_H
#define ELLIPSE_H

#include "cad/const.h"

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
    class Ellipse : public CADEntity, public geo::Ellipse {
        public:
            Ellipse(const geo::Coordinate& center, const geo::Coordinate& majorP, double minorRadius, double startAngle, double endAngle);
            Ellipse(const geo::Coordinate& center, const geo::Coordinate& majorP, double minorRadius, double startAngle, double endAngle, const QList<MetaTypePtr>& metaTypes);
    };
    typedef shared_ptr<const lc::Ellipse> EllipsePtr;
}

#endif // ELLIPSE_H
