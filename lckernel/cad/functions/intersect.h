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

namespace lc {

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

        bool operator()(const lc::geo::Vector &, const lc::geo::Vector &);
        bool operator()(const lc::geo::Vector &, const lc::entity::Point &);
        bool operator()(const lc::geo::Vector &, const lc::entity::Line &);
        bool operator()(const lc::geo::Vector &, const lc::entity::Circle &);
        bool operator()(const lc::geo::Vector &, const lc::entity::Arc &);
        bool operator()(const lc::geo::Vector &, const lc::entity::Ellipse &);
        bool operator()(const lc::geo::Vector &, const lc::entity::Spline &);
        bool operator()(const lc::geo::Vector &, const lc::entity::LWPolyline &);
        bool operator()(const lc::geo::Vector &, const lc::entity::Image &){return false;};


        bool operator()(const lc::entity::Line &, const lc::geo::Vector &);
        bool operator()(const lc::entity::Line &, const lc::entity::Point &);
        bool operator()(const lc::entity::Line &, const lc::entity::Line &);
        bool operator()(const lc::entity::Line &, const lc::entity::Circle &);
        bool operator()(const lc::entity::Line &, const lc::entity::Arc &);
        bool operator()(const lc::entity::Line &, const lc::entity::Ellipse &);
        bool operator()(const lc::entity::Line &, const lc::entity::Spline &);
        bool operator()(const lc::entity::Line &, const lc::entity::LWPolyline &);
        bool operator()(const lc::entity::Line &, const lc::entity::Image &){return false;};


        bool operator()(const lc::entity::Point &, const lc::geo::Vector &);
        bool operator()(const lc::entity::Point &, const lc::entity::Point &);
        bool operator()(const lc::entity::Point &, const lc::entity::Line &);
        bool operator()(const lc::entity::Point &, const lc::entity::Circle &);
        bool operator()(const lc::entity::Point &, const lc::entity::Arc &);
        bool operator()(const lc::entity::Point &, const lc::entity::Ellipse &);
        bool operator()(const lc::entity::Point &, const lc::entity::Spline &);
        bool operator()(const lc::entity::Point &, const lc::entity::LWPolyline &);
        bool operator()(const lc::entity::Point &, const lc::entity::Image &){return false;};


        bool operator()(const lc::entity::Circle &, const lc::geo::Vector &);
        bool operator()(const lc::entity::Circle &, const lc::entity::Point &);
        bool operator()(const lc::entity::Circle &, const lc::entity::Line &);
        bool operator()(const lc::entity::Circle &, const lc::entity::Circle &);
        bool operator()(const lc::entity::Circle &, const lc::entity::Arc &);
        bool operator()(const lc::entity::Circle &, const lc::entity::Ellipse &);
        bool operator()(const lc::entity::Circle &, const lc::entity::Spline &);
        bool operator()(const lc::entity::Circle &, const lc::entity::LWPolyline &);
        bool operator()(const lc::entity::Circle &, const lc::entity::Image &){return false;};


        bool operator()(const lc::entity::Arc &, const lc::geo::Vector &);
        bool operator()(const lc::entity::Arc &, const lc::entity::Point &);
        bool operator()(const lc::entity::Arc &, const lc::entity::Line &);
        bool operator()(const lc::entity::Arc &, const lc::entity::Circle &);
        bool operator()(const lc::entity::Arc &, const lc::entity::Arc &);
        bool operator()(const lc::entity::Arc &, const lc::entity::Ellipse &);
        bool operator()(const lc::entity::Arc &, const lc::entity::Spline &);
        bool operator()(const lc::entity::Arc &, const lc::entity::LWPolyline &);
        bool operator()(const lc::entity::Arc &, const lc::entity::Image &){return false;};


        bool operator()(const lc::entity::Ellipse &, const lc::geo::Vector &);
        bool operator()(const lc::entity::Ellipse &, const lc::entity::Point &);
        bool operator()(const lc::entity::Ellipse &, const lc::entity::Line &);
        bool operator()(const lc::entity::Ellipse &, const lc::entity::Circle &);
        bool operator()(const lc::entity::Ellipse &, const lc::entity::Arc &);
        bool operator()(const lc::entity::Ellipse &, const lc::entity::Ellipse &);
        bool operator()(const lc::entity::Ellipse &, const lc::entity::Spline &);
        bool operator()(const lc::entity::Ellipse &, const lc::entity::LWPolyline &);
        bool operator()(const lc::entity::Ellipse &, const lc::entity::Image &){return false;};


        bool operator()(const lc::entity::Spline &, const lc::geo::Vector &);
        bool operator()(const lc::entity::Spline &, const lc::entity::Point &);
        bool operator()(const lc::entity::Spline &, const lc::entity::Line &);
        bool operator()(const lc::entity::Spline &, const lc::entity::Circle &);
        bool operator()(const lc::entity::Spline &, const lc::entity::Arc &);
        bool operator()(const lc::entity::Spline &, const lc::entity::Ellipse &);
        bool operator()(const lc::entity::Spline &, const lc::entity::Spline &);
        bool operator()(const lc::entity::Spline &, const lc::entity::LWPolyline &);
        bool operator()(const lc::entity::Spline &, const lc::entity::Image &){return false;};


        bool operator()(const lc::entity::LWPolyline &, const lc::geo::Vector &);
        bool operator()(const lc::entity::LWPolyline &, const lc::entity::Point &);
        bool operator()(const lc::entity::LWPolyline &, const lc::entity::Line &);
        bool operator()(const lc::entity::LWPolyline &, const lc::entity::Circle &);
        bool operator()(const lc::entity::LWPolyline &, const lc::entity::Arc &);
        bool operator()(const lc::entity::LWPolyline &, const lc::entity::Ellipse &);
        bool operator()(const lc::entity::LWPolyline &, const lc::entity::Spline &);
        bool operator()(const lc::entity::LWPolyline &, const lc::entity::LWPolyline &);
        bool operator()(const lc::entity::LWPolyline &, const lc::entity::Image &){return false;};


        bool operator()(const lc::entity::Image &, const lc::geo::Vector &){return false;};
        bool operator()(const lc::entity::Image &, const lc::entity::Point &){return false;};
        bool operator()(const lc::entity::Image &, const lc::entity::Line &){return false;};
        bool operator()(const lc::entity::Image &, const lc::entity::Circle &){return false;};
        bool operator()(const lc::entity::Image &, const lc::entity::Arc &){return false;};
        bool operator()(const lc::entity::Image &, const lc::entity::Ellipse &){return false;};
        bool operator()(const lc::entity::Image &, const lc::entity::Spline &){return false;};
        bool operator()(const lc::entity::Image &, const lc::entity::LWPolyline &){return false;};
        bool operator()(const lc::entity::Image &, const lc::entity::Image &){return false;};

        bool operator()(const lc::Visitable &s1, const lc::Visitable &s2) {
            // If we end up here we found a un-supported intersection
            // std::cout<<typeid(s1).name()<<"\t"<< quote(s1) <<" - ";
            // std::cout<<typeid(s2).name()<<"\t"<< quote(s2) <<"\n";
            return false;
        }

        template<typename S>
        bool operator()(S &s1, S &s2) {
//                s1.accept(printer);
            std::cout << " No operator found for this Intersection. \n";
//                s2.accept(printer);
            std::cout << std::endl;
            return true;
        }

        std::vector<geo::Coordinate> result() const;

    private:
        void geovisit(const geo::Vector &, const geo::Vector &);
        void geovisit(const geo::Vector &, const geo::Arc &);
        void geovisit(const geo::Arc &, const geo::Arc &);
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
        IntersectMany(std::vector<entity::CADEntity_CSPtr>, Intersect::Method = Intersect::OnEntity, double tolerance = LCTOLERANCE);

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
