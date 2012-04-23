#ifndef LINE_H
#define LINE_H

#include "cad/const.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geovector.h"
#include "cad/base/cadentity.h"
#include "cad/interface/snapable.h"
#include "cad/vo/entitycoordinate.h"

namespace lc {

    /*!
     * \brief A line that can be put in a drawing
     *
     * A lc::line is a graphics line item that can be put on a drawing using a CreateEntity operation.
     *
     * \sa lc::CreateEntities::append
     * \author R. van Twisk
     * \date 2012-04-16
     */
    class Line : public CADEntity, public geo::Vector, public Snapable {
        public:
            /*!
             * \brief Construct a new line
             *
             * \param start,end Coordinate the line should start and end from
             */
            Line(const geo::Coordinate& start, const geo::Coordinate& end);

            /*!
             * \brief Construct a new line
             *
             * \param start,end Coordinate the line should start and end from
             * \param metaTypes A list of metatypes associated with this line
             * \sa lc::Color
             * \sa lc::LineWidth
             * \sa lc::MetaType
             */
            Line(const geo::Coordinate& start, const geo::Coordinate& end, const QList<MetaTypePtr>& metaTypes);

        public:
            /*!
             * \brief Find a number of snap points the line has available
             * This function returns a ordered list, closest to \em coord and can return multiple snap points
             *
             * \param coord Coordinate to lookup the nearest coordinate from
             * \param minDistanceToSnap Minimum distance to the path to
             * \param maxNumberOfSnapPoints Maximum number of snappoints that have to be looked up
             *
             * \sa lc::EntityCoordinate
             */
            virtual QList<lc::EntityCoordinate> snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const;

            /*!
             * \brief Find the nearest point on the path for this entity for the coordinate \em coord
             *
             * \param coord Coordinate to lookup the nearest coordinate from
             * \sa lc::CADEntity
             */
            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const;
        private:
    };

    /*!
     * \typedef shared pointer to a lc::Line
     * \sa lc::Line
     */
    typedef shared_ptr<const lc::Line> LinePtr;
}
#endif // LINE_H
