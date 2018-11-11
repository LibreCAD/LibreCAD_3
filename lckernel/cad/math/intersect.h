#pragma once

#include "cad/primitive/arc.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/dimaligned.h"
#include "cad/primitive/dimangular.h"
#include "cad/primitive/dimdiametric.h"
#include "cad/primitive/dimlinear.h"
#include "cad/primitive/dimradial.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/line.h"
#include "cad/primitive/lwpolyline.h"
#include "cad/primitive/point.h"
#include "cad/primitive/spline.h"
#include "cad/primitive/text.h"
#include "cad/primitive/image.h"

#include "cad/base/cadentity.h"
#include "cad/geometry/geoarc.h"
#include "cad/geometry/geoarea.h"
#include "cad/geometry/geobase.h"
#include "cad/geometry/geocircle.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geoellipse.h"
#include "cad/geometry/geospline.h"
#include "cad/geometry/geovector.h"
#include <vector>

//TODO: operator() parameters should be inverted automatically

namespace lc {
    namespace maths {

        /**
          * @brief calculate the intersection pojnts of 2 entities
          * @sa IntersectMany
          */
        struct Intersect {
            public:
                enum Method {
                    OnEntity = 0,     // Means that the location must be on both entities
                    OnPath = 1       // means that the paths may intersect outside of the real path.
                    // For example two lines in a slight angle might intersect outside of line's Area
                    // When method == Any is selected, the system will return that coordinate, otherwise
                    // the point must be on both

                    // Test = 2 // I am considering to add a 'Test' option, this would allow to set a boolean if
                    // a intersection would have happened. This would potentially speed up line/line intersections
                    // during area selection where usually CAD drawings do contain a lot of lines.
                    // THis would also help during selection of LWPolylines where we can stop testing if
                    // any point would have been found.

                };
                Intersect(Method method, double tolerance);

                bool operator()(const lc::geo::Vector& v1, const lc::geo::Vector& v2);
                bool operator()(const lc::geo::Vector& v, const lc::entity::Point& p);
                bool operator()(const lc::geo::Vector& v, const lc::geo::Circle& c);
                bool operator()(const lc::geo::Vector& v, const lc::entity::Arc& a);
                bool operator()(const lc::geo::Vector& v, const lc::entity::Ellipse& e);
                bool operator()(const lc::geo::Vector& v, const lc::entity::Spline& s);
                bool operator()(const lc::geo::Vector& v, const lc::entity::LWPolyline& lwp);

                bool operator()(const lc::entity::Point& p, const lc::geo::Vector& v){return (*this)(v,p);};
                bool operator()(const lc::entity::Point& p1, const lc::entity::Point& p2);
                bool operator()(const lc::entity::Point& p, const lc::geo::Circle& c);
                bool operator()(const lc::entity::Point& p, const lc::entity::Arc& a);
                bool operator()(const lc::entity::Point& p, const lc::entity::Ellipse& e);
                bool operator()(const lc::entity::Point& p, const lc::entity::Spline& s);
                bool operator()(const lc::entity::Point& p, const lc::entity::LWPolyline& lwp);

                bool operator()(const lc::geo::Circle& c, const lc::geo::Vector& v){return (*this)(v,c);};
                bool operator()(const lc::geo::Circle& c, const lc::entity::Point& p){return (*this)(p,c);};
                bool operator()(const lc::geo::Circle& c1, const lc::geo::Circle& c2);
                bool operator()(const lc::geo::Circle& c, const lc::entity::Arc& arc);
                bool operator()(const lc::geo::Circle& c, const lc::entity::Ellipse& e);
                bool operator()(const lc::geo::Circle& c, const lc::entity::Spline& s);
                bool operator()(const lc::geo::Circle& c, const lc::entity::LWPolyline& lwp);

                bool operator()(const lc::entity::Arc& a, const lc::geo::Vector& v){return (*this)(v,a);};
                bool operator()(const lc::entity::Arc& a, const lc::entity::Point& p){return (*this)(p,a);};
                bool operator()(const lc::entity::Arc& a, const lc::geo::Circle& c){return (*this)(c,a);};
                bool operator()(const lc::entity::Arc& a1, const lc::entity::Arc& a2);
                bool operator()(const lc::entity::Arc& a, const lc::entity::Ellipse& e);
                bool operator()(const lc::entity::Arc& a, const lc::entity::Spline& s);
                bool operator()(const lc::entity::Arc& a, const lc::entity::LWPolyline& lwp);

                bool operator()(const lc::entity::Ellipse& e, const lc::geo::Vector& v){return (*this)(v,e);};
                bool operator()(const lc::entity::Ellipse& e, const lc::entity::Point& p){return (*this)(p,e);};
                bool operator()(const lc::entity::Ellipse& e, const lc::geo::Circle& c){return (*this)(c,e);};
                bool operator()(const lc::entity::Ellipse& e, const lc::entity::Arc& a){return (*this)(a,e);};
                bool operator()(const lc::entity::Ellipse& e1, const lc::entity::Ellipse& e2);
                bool operator()(const lc::entity::Ellipse& e, const lc::entity::Spline& s);
                bool operator()(const lc::entity::Ellipse& e, const lc::entity::LWPolyline& lwp);

                bool operator()(const lc::entity::Spline& s, const lc::geo::Vector& v){return (*this)(v,s);};
                bool operator()(const lc::entity::Spline& s, const lc::entity::Point& p){return (*this)(p,s);};
                bool operator()(const lc::entity::Spline& s, const lc::geo::Circle& c){return (*this)(c,s);};
                bool operator()(const lc::entity::Spline& s, const lc::entity::Arc& a){return (*this)(a,s);};
                bool operator()(const lc::entity::Spline& s, const lc::entity::Ellipse& e){return (*this)(e,s);};
                bool operator()(const lc::entity::Spline& s1, const lc::entity::Spline& s2);
                bool operator()(const lc::entity::Spline& s, const lc::entity::LWPolyline& lwp);

                bool operator()(const lc::entity::LWPolyline& lwp, const lc::geo::Vector& v){return (*this)(v,lwp);};
                bool operator()(const lc::entity::LWPolyline& lwp, const lc::entity::Point& p){return (*this)(p,lwp);};
                bool operator()(const lc::entity::LWPolyline& lwp, const lc::geo::Circle& c){return (*this)(c,lwp);};
                bool operator()(const lc::entity::LWPolyline& lwp, const lc::entity::Arc& a){return (*this)(a,lwp);};
                bool operator()(const lc::entity::LWPolyline& lwp, const lc::entity::Ellipse& e){return (*this)(e,lwp);};
                bool operator()(const lc::entity::LWPolyline& lwp, const lc::entity::Spline& s){return (*this)(s,lwp);};
                bool operator()(const lc::entity::LWPolyline& lwp1, const lc::entity::LWPolyline& lwp2);

                bool operator()(const lc::Visitable& s1, const lc::Visitable& s2) {
                    // If we end up here we found a un-supported intersection .. like dimensions...
                    return false;
                }
                //Do not use template for overload here..

                std::vector<geo::Coordinate> result() const;

            private:
                void geovisit(const geo::Vector& v1, const geo::Vector& v2);

                void geovisit(const geo::Vector& v, const geo::Arc& arc);

                void geovisit(const geo::Arc& arc1, const geo::Arc& arc2);

            private:
                std::vector<geo::Coordinate> _intersectionPoints;
                const Method _method;
                const double _tolerance;
        };

        /**
          * @brief calculate intersection points of many entities
          * @note Can we make this into a general template ???
          * @sa Intersect
          */
        class IntersectMany {
            public:
                IntersectMany(std::vector<entity::CADEntity_CSPtr>, Intersect::Method = Intersect::OnEntity,
                              double tolerance = LCTOLERANCE);

                std::vector<geo::Coordinate> result() const;

            private:
                std::vector<entity::CADEntity_CSPtr> _entities;
                const Intersect::Method _method;
                const double _tolerance;
        };

        /**
          * @brief calculate intersection points of many entities
          * @note Can we make this into a general template ???
          * @sa Intersect
          */
        class IntersectAgainstOthers {
            public:
                IntersectAgainstOthers(std::vector<entity::CADEntity_CSPtr>, std::vector<entity::CADEntity_CSPtr>,
                                       Intersect::Method, double tolerance);

                std::vector<geo::Coordinate> result() const;

            private:
                std::vector<entity::CADEntity_CSPtr> _entities;
                std::vector<entity::CADEntity_CSPtr> _others;
                const Intersect::Method _method;
                const double _tolerance;
        };

        /**
          * @brief calculate intersection points of many entities but beal out asap when a intersection point was found
          * @note Can we make this into a general template ???
          * @sa Intersect
          */
        class HasIntersectAgainstOthers {
            public:
                HasIntersectAgainstOthers(std::vector<entity::CADEntity_CSPtr>, std::vector<entity::CADEntity_CSPtr>,
                                          Intersect::Method, double tolerance);

                bool result() const;

            private:
                std::vector<entity::CADEntity_CSPtr> _entities;
                std::vector<entity::CADEntity_CSPtr> _others;
                const Intersect::Method _method;
                const double _tolerance;
        };
    }
}
