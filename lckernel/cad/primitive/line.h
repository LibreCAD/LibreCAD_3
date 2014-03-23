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
    class Line : public enable_shared_from_this<Line>, public CADEntity, public geo::Vector, public Snapable {
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
            Line(const geo::Coordinate& start, const geo::Coordinate& end, const QList<shared_ptr<const lc::MetaType> >& metaTypes);

            /*!
             * \brief Construct a new line
             *
             * \param vector Coordinate the line should start and end from
             * \param metaTypes A list of metatypes associated with this line
             * \sa lc::Color
             * \sa lc::LineWidth
             * \sa lc::MetaType
             */
            Line(const geo::Vector& vector, const QList<shared_ptr<const lc::MetaType> >& metaTypes);

        public:
            virtual QList<lc::EntityCoordinate> snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const;
            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const;
            virtual shared_ptr<const lc::CADEntity> move(const geo::Coordinate& offset) const;
            virtual shared_ptr<const lc::CADEntity> rotate(const double& angle) const;
        public:
            virtual void accept(shared_ptr<const lc::Line> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const lc::Circle> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const lc::Arc> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const lc::Ellipse> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const lc::Text> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const lc::CADEntity> o, EntityVisitor& ei) const {
                o->accept(shared_from_this(), ei);
            }
            virtual void dispatch(EntityDispatch& ed) const {
                ed.visit(shared_from_this());
            }

        private:
    };

}
#endif // LINE_H
