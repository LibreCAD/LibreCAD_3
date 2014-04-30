#ifndef LINE_H
#define LINE_H

#include "cad/const.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"

#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geovector.h"
#include "cad/base/cadentity.h"
#include "cad/interface/snapable.h"
#include "cad/vo/entitycoordinate.h"

#include <cad/meta/layer.h>

namespace lc {

    /*!
     * \brief A line that can be put in a drawing
     *
     * A line is a graphics line item that can be put on a drawing using a CreateEntity operation.
     *
     * \sa CreateEntities::append
     * \author R. van Twisk
     * \date 2012-04-16
     */
    class Line : public std::enable_shared_from_this<Line>, public CADEntity, public geo::Vector, public Snapable {
        public:
            /*!
             * \brief Construct a new line
             *
             * \param start,end Coordinate the line should start and end from
             */
            Line(const geo::Coordinate& start, const geo::Coordinate& end, const std::shared_ptr<const Layer> layer);

            /*!
             * \brief Construct a new line
             *
             * \param start,end Coordinate the line should start and end from
             * \param metaTypes A list of metatypes associated with this line
             * \sa Color
             * \sa LineWidth
             * \sa MetaType
             */
            Line(const geo::Coordinate& start, const geo::Coordinate& end, const std::shared_ptr<const Layer> layer, const QList<std::shared_ptr<const MetaType> >& metaTypes);

            /*!
             * \brief Construct a new line
             *
             * \param vector Coordinate the line should start and end from
             * \param metaTypes A list of metatypes associated with this line
             * \sa Color
             * \sa LineWidth
             * \sa MetaType
             */
            Line(const geo::Vector& vector, const std::shared_ptr<const Layer> layer, const QList<std::shared_ptr<const MetaType> >& metaTypes);

        public:
            virtual QList<EntityCoordinate> snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const;
            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const;

        public:
            virtual std::shared_ptr<const CADEntity> move(const geo::Coordinate& offset) const;
            virtual std::shared_ptr<const CADEntity> copy(const geo::Coordinate& offset) const;
            virtual std::shared_ptr<const CADEntity> rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const;
            virtual std::shared_ptr<const CADEntity> scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const;


        public:
            virtual void accept(std::shared_ptr<const Line> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(std::shared_ptr<const Circle> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(std::shared_ptr<const Arc> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(std::shared_ptr<const Ellipse> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(std::shared_ptr<const Text> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(std::shared_ptr<const Spline> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(std::shared_ptr<const CADEntity> o, EntityVisitor& ei) const {
                o->accept(shared_from_this(), ei);
            }
            virtual void dispatch(EntityDispatch& ed) const {
                ed.visit(shared_from_this());
            }

        private:
    };

}
#endif // LINE_H
