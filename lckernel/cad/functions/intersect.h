#pragma once

#include "cad/base/cadentity.h"
#include "cad/geometry/geocoordinate.h"
#include <vector>

namespace lc {

    /**
      * @brief calculate the intersection pojnts of 2 entities
      * @sa IntersectMany
      */
    struct Intersect {
    public:
        enum Method {
            OnPath = 0,     // means that the paths must intersect
            Any = 1       // means that the paths may intersect outside of the real path.
            // For example two lines in a slight angle might intersect outside of line's Area
            // When method == Any is selected, the system will return that coordinate, otherwhise
            // the point must be on both

            // Test = 2 // I am considering to add a 'Test' option, this would allow to set a boolean if
            // a intersection would have happened. This would potentially speed up line/line intersections
            // during area selection where usually CAD drawings do contain a lot of lines.
            // THis would also help during selection of LWPolylines where we can stop testing if
            // any point would have been found.

        };

        Intersect(Method method, double tolerance);

        bool operator()(lc::geo::Vector &, lc::geo::Vector &);
        bool operator()(lc::geo::Vector &, lc::entity::Point &);
        bool operator()(lc::geo::Vector &, lc::entity::Line &);
        bool operator()(lc::geo::Vector &, lc::entity::Circle &);
        bool operator()(lc::geo::Vector &, lc::entity::Arc &);
        bool operator()(lc::geo::Vector &, lc::entity::Ellipse &);
        bool operator()(lc::geo::Vector &, lc::entity::Spline &);
        bool operator()(lc::geo::Vector &, lc::entity::LWPolyline &);


        bool operator()(lc::entity::Line &, lc::geo::Vector &);
        bool operator()(lc::entity::Line &, lc::entity::Point &);
        bool operator()(lc::entity::Line &, lc::entity::Line &);
        bool operator()(lc::entity::Line &, lc::entity::Circle &);
        bool operator()(lc::entity::Line &, lc::entity::Arc &);
        bool operator()(lc::entity::Line &, lc::entity::Ellipse &);
        bool operator()(lc::entity::Line &, lc::entity::Spline &);
        bool operator()(lc::entity::Line &, lc::entity::LWPolyline &);


        bool operator()(lc::entity::Point &, lc::geo::Vector &);
        bool operator()(lc::entity::Point &, lc::entity::Point &);
        bool operator()(lc::entity::Point &, lc::entity::Line &);
        bool operator()(lc::entity::Point &, lc::entity::Circle &);
        bool operator()(lc::entity::Point &, lc::entity::Arc &);
        bool operator()(lc::entity::Point &, lc::entity::Ellipse &);
        bool operator()(lc::entity::Point &, lc::entity::Spline &);
        bool operator()(lc::entity::Point &, lc::entity::LWPolyline &);


        bool operator()(lc::entity::Circle &, lc::geo::Vector &);
        bool operator()(lc::entity::Circle &, lc::entity::Point &);
        bool operator()(lc::entity::Circle &, lc::entity::Line &);
        bool operator()(lc::entity::Circle &, lc::entity::Circle &);
        bool operator()(lc::entity::Circle &, lc::entity::Arc &);
        bool operator()(lc::entity::Circle &, lc::entity::Ellipse &);
        bool operator()(lc::entity::Circle &, lc::entity::Spline &);
        bool operator()(lc::entity::Circle &, lc::entity::LWPolyline &);


        bool operator()(lc::entity::Arc &, lc::geo::Vector &);
        bool operator()(lc::entity::Arc &, lc::entity::Point &);
        bool operator()(lc::entity::Arc &, lc::entity::Line &);
        bool operator()(lc::entity::Arc &, lc::entity::Circle &);
        bool operator()(lc::entity::Arc &, lc::entity::Arc &);
        bool operator()(lc::entity::Arc &, lc::entity::Ellipse &);
        bool operator()(lc::entity::Arc &, lc::entity::Spline &);
        bool operator()(lc::entity::Arc &, lc::entity::LWPolyline &);


        bool operator()(lc::entity::Ellipse &, lc::geo::Vector &);
        bool operator()(lc::entity::Ellipse &, lc::entity::Point &);
        bool operator()(lc::entity::Ellipse &, lc::entity::Line &);
        bool operator()(lc::entity::Ellipse &, lc::entity::Circle &);
        bool operator()(lc::entity::Ellipse &, lc::entity::Arc &);
        bool operator()(lc::entity::Ellipse &, lc::entity::Ellipse &);
        bool operator()(lc::entity::Ellipse &, lc::entity::Spline &);
        bool operator()(lc::entity::Ellipse &, lc::entity::LWPolyline &);


        bool operator()(lc::entity::Spline &, lc::geo::Vector &);
        bool operator()(lc::entity::Spline &, lc::entity::Point &);
        bool operator()(lc::entity::Spline &, lc::entity::Line &);
        bool operator()(lc::entity::Spline &, lc::entity::Circle &);
        bool operator()(lc::entity::Spline &, lc::entity::Arc &);
        bool operator()(lc::entity::Spline &, lc::entity::Ellipse &);
        bool operator()(lc::entity::Spline &, lc::entity::Spline &);
        bool operator()(lc::entity::Spline &, lc::entity::LWPolyline &);


        bool operator()(lc::entity::LWPolyline &, lc::geo::Vector &);
        bool operator()(lc::entity::LWPolyline &, lc::entity::Point &);
        bool operator()(lc::entity::LWPolyline &, lc::entity::Line &);
        bool operator()(lc::entity::LWPolyline &, lc::entity::Circle &);
        bool operator()(lc::entity::LWPolyline &, lc::entity::Arc &);
        bool operator()(lc::entity::LWPolyline &, lc::entity::Ellipse &);
        bool operator()(lc::entity::LWPolyline &, lc::entity::Spline &);
        bool operator()(lc::entity::LWPolyline &, lc::entity::LWPolyline &);


        bool operator()(lc::Visitable &s1, lc::Visitable &s2) {
            // If we end up here we found a un-supported intersection
            // std::cout<<typeid(s1).name()<<"\t"<< quote(s1) <<" - ";
            // std::cout<<typeid(s2).name()<<"\t"<< quote(s2) <<"\n";
            return false;
        }

        template<typename S>
        bool operator()(S &s1, S &s2) {
//                s1.accept(printer);
            std::cout << " == ";
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
        IntersectMany(std::vector<entity::CADEntity_CSPtr>, Intersect::Method, double tolerance);

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
