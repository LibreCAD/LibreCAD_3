#include "intersect.h"
#include "cad/math/intersectionhandler.h"

using namespace lc::maths;


Intersect::Intersect(Method method, double tolerance) :
        _method(method),
        _tolerance(tolerance) {
}

std::vector<lc::geo::Coordinate> Intersect::result() const {
    return _intersectionPoints;
}

// Vector
bool Intersect::operator()(const lc::geo::Vector& v1, const lc::geo::Vector& v2) {
    geovisit(v1, v2);
    return false;
}

bool Intersect::operator()(const lc::geo::Vector& v, const lc::geo::Circle& c) {
    geovisit(v, lc::geo::Arc(c.center(), c.radius(), -M_PI, M_PI));
    return false;
}

bool Intersect::operator()(const lc::geo::Vector& v, const lc::geo::Arc& a) {
    geovisit(v, a);
    return false;
}

bool Intersect::operator()(const lc::geo::Vector& v, const lc::geo::Ellipse& e) {
	geovisit(v, e);
	return false;
}

bool Intersect::operator()(const lc::geo::Vector& v, const lc::geo::Spline& s) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::geo::Vector& v, const lc::entity::LWPolyline& lwp) {
    auto list1 = lwp.asEntities();

    // Note: The dynamic_pointer_cast won't win a beauty contest, but the plan is to split
    // the EntityVisitor into a GeoVisitor and EntityVisitor such that a application deciding
    // to use double dispatch can decide to use a specific implementation.
    // Once added, we can get rid of the dynamic_pointer_casts and simply
    // call entity1.visit(entity2);
    for (auto &entity1 : list1) {
        if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity1)) {
            geovisit(v, *arc.get());
        } else {
            geovisit(v, *std::dynamic_pointer_cast<const lc::geo::Vector>(entity1).get());
        }
    }
    return false;
}

// Point
bool Intersect::operator()(const lc::geo::Coordinate& p1, const lc::geo::Coordinate& p2) {
    if(p1.distanceTo(p2) < LCTOLERANCE) _intersectionPoints.push_back(p1);
    return false;
}

// Circle
bool Intersect::operator()(const lc::geo::Circle& c1, const lc::geo::Circle& c2) {
    auto &&coords = maths::Intersection::QuadQuad(c1.equation(), c2.equation());
    for (auto &i : coords) {
        _intersectionPoints.push_back(i);
    }
    return false;
}

bool Intersect::operator()(const lc::geo::Circle& c, const lc::geo::Arc& arc) {
    auto &&coords = maths::Intersection::QuadQuad(c.equation(), arc.equation());
    if (_method == Method::OnPath) {
        _intersectionPoints.reserve(_intersectionPoints.size() + coords.size());
        _intersectionPoints.insert(coords.end(), coords.begin(), coords.end());
    } else {
        for (auto &point : coords) {
            double a = (point - arc.center()).angle();
            if (arc.isAngleBetween(a)) {
                _intersectionPoints.push_back(point);
            }
        }
    }
    return false;
}

bool Intersect::operator()(const lc::geo::Circle& c, const lc::geo::Ellipse& ellipse) {
    auto &&coords = maths::Intersection::QuadQuad(ellipse.equation(), ellipse.equation());
    if (_method == Method::OnPath) {
        _intersectionPoints.reserve(_intersectionPoints.size() + coords.size());
        _intersectionPoints.insert(coords.end(), coords.begin(), coords.end());
    } else {
        for (auto &point : coords) {
            double a = (point - ellipse.center()).angle();
            if (ellipse.isAngleBetween(a)) {
                _intersectionPoints.push_back(point);
            }
        }
    }
    return false;
}

bool Intersect::operator()(const lc::geo::Circle& c, const lc::geo::Spline& s) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false; //visit(c, s);
}

bool Intersect::operator()(const lc::geo::Circle&c, const lc::entity::LWPolyline& lwp) {
    auto &list1 = lwp.asEntities();
    auto a = lc::geo::Arc(c.center(), c.radius(), -M_PI, M_PI);
    // Note: The dynamic_pointer_cast won't win a beauty contest, but the plan is to split
    // the EntityVisitor into a GeoVisitor and EntityVisitor such that a application deciding
    // to use double dispatch can decide to use a specific implementation.
    // Once added, we can get rid of the dynamic_pointer_casts and simply
    // call entity1.visit(entity2);
    for (auto &entity1 : list1) {
        if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity1)) {
            geovisit(a, *arc.get());
        } else {
            geovisit(*std::dynamic_pointer_cast<const lc::geo::Vector>(entity1).get(), a);
        }
    }
    return false;
}



// ARC
// ----------------
//bool Intersect::operator()(const lc::geo::Arc& a, const lc::geo::Vector& v) {
//    geovisit(v, a);
//    return false;

    /* Please do not delete this for the moment
    const geo::Coordinate nearest = line.nearestPointOnPath(arc.center());
    double dist = arc.center().distanceTo(nearest);

    // special case: arc touches line (tangent):
    // TODO: We properly should add a tolorance here ??
    if (fabs(dist - arc.radius()) < _tolerance) {
        _intersectionPoints.push_back(nearest);
        return;
    }

    const geo::Coordinate d = line.end() - line.start();
    const double r = arc.radius();
    const geo:: Coordinate delta = line.start() - arc.center();
    const double d2 = d.squared();

    //intersection
    // solution = p + t d;
    //| p -c+ t d|^2 = r^2
    // |d|^2 t^2 + 2 (p-c).d t + |p-c|^2 -r^2 = 0
    const double a1 = delta.dot(d);
    const double discriminant = a1 * a1 - d2 * (delta.squared() - r * r);

    if (discriminant < - _tolerance) {
        return;
    } else {
        const double t = sqrtf(fabs(discriminant));
        //two intersections
        const geo::Coordinate c1(line.start() + d * (t - a1) / d2);
        const geo::Coordinate c2(line.start() - d * (t + a1) / d2);

        if (_method == Method::OnPath || (_method == Method::OnEntity && arc.isCoordinateOnPath(c1) && line.isCoordinateOnPath(c1))) {
            _intersectionPoints.push_back(c1);
        }

        if (_method == Method::OnPath || (_method == Method::OnEntity && arc.isCoordinateOnPath(c2) && line.isCoordinateOnPath(c2))) {
            _intersectionPoints.push_back(c2);
        }
    } */
//}
//----------- cont..
bool Intersect::operator()(const lc::geo::Arc& a1, const lc::geo::Arc& a2) {
    geovisit(a1, a2);
    return false;
}

bool Intersect::operator()(const lc::geo::Arc& a, const lc::geo::Ellipse& e) {
    geovisit(a, e);
    return false;
}

bool Intersect::operator()(const lc::geo::Arc& a, const lc::geo::Spline& s) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::geo::Arc& a, const lc::entity::LWPolyline& lwp) {
    auto &list1 = lwp.asEntities();
    // Note: The dynamic_pointer_cast won't win a beauty contest, but the plan is to split
    // the EntityVisitor into a GeoVisitor and EntityVisitor such that a application deciding
    // to use double dispatch can decide to use a specific implementation.
    // Once added, we can get rid of the dynamic_pointer_casts and simply
    // call entity1.visit(entity2);
    for (auto &entity1 : list1) {
        if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity1)) {
            geovisit(a, *arc.get());
        }
        else {
            geovisit(*std::dynamic_pointer_cast<const lc::geo::Vector>(entity1).get(), a);
        }
    }
    return false; //visit(l1, a1);
}

// Ellipse
bool Intersect::operator()(const lc::geo::Ellipse& e1, const lc::geo::Ellipse& e2) {
    geovisit(e1, e2);
    return false;
}

bool Intersect::operator()(const lc::geo::Ellipse& e, const lc::geo::Spline& s) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::geo::Ellipse& e, const lc::entity::LWPolyline& lwp) {
    auto &list1 = lwp.asEntities();
    // Note: The dynamic_pointer_cast won't win a beauty contest, but the plan is to split
    // the EntityVisitor into a GeoVisitor and EntityVisitor such that a application deciding
    // to use double dispatch can decide to use a specific implementation.
    // Once added, we can get rid of the dynamic_pointer_casts and simply
    // call entity1.visit(entity2);
    for (auto &entity1 : list1) {
        if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity1)) {
            geovisit(e, *arc.get());
        }
        else {
            geovisit(*std::dynamic_pointer_cast<const lc::geo::Vector>(entity1).get(), e);
        }
    }
    return false; //visit(l1, a1);
}

// Spline
bool Intersect::operator()(const lc::geo::Spline& s1, const lc::geo::Spline& s2) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

bool Intersect::operator()(const lc::geo::Spline& s, const lc::entity::LWPolyline& lwp) {
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;
    return false;
}

// LWPolyline
bool Intersect::operator()(const lc::entity::LWPolyline& lwp1, const lc::entity::LWPolyline& lwp2) {
    auto &list1 = lwp1.asEntities();
    auto &list2 = lwp2.asEntities();

    // Note: The dynamic_pointer_cast won't win a beauty contest, but the plan is to split
    // the EntityVisitor into a GeoVisitor and EntityVisitor such that a application deciding
    // to use double dispatch can decide to use a specific implementation.
    // Once added, we can get rid of the dynamic_pointer_casts and simply
    // call entity1.visit(entity2);
    for (auto &entity1 : list1) {
        for (auto &entity2 : list2) {
            if (auto vector = std::dynamic_pointer_cast<const lc::geo::Vector>(entity1)) {
                if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity2)) {
                    geovisit(*vector.get(), *arc.get());
                }
                else {
                    geovisit(*vector.get(), *std::dynamic_pointer_cast<const lc::geo::Vector>(entity2).get());
                }
            }
            else {
                if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(entity2)) {
                    geovisit(*std::dynamic_pointer_cast<const lc::geo::Arc>(entity1).get(), *arc.get());
                }
                else {
                    geovisit(*std::dynamic_pointer_cast<const lc::geo::Vector>(entity2).get(),
                             *std::dynamic_pointer_cast<const lc::geo::Arc>(entity1).get());
                }
            }
        }
    }
    return false;
}


void Intersect::geovisit(const geo::Vector& v1, const geo::Vector& v2) {
    const geo::Coordinate p1 = v1.start();
    const geo::Coordinate p2 = v1.end();
    const geo::Coordinate p3 = v2.start();
    const geo::Coordinate p4 = v2.end();

    const double num = ((p4.x() - p3.x()) * (p1.y() - p3.y()) - (p4.y() - p3.y()) * (p1.x() - p3.x()));
    const double div = ((p4.y() - p3.y()) * (p2.x() - p1.x()) - (p4.x() - p3.x()) * (p2.y() - p1.y()));

    // TODO: We properly should add a tolorance here ??
    if (std::abs(div) > _tolerance) {
        double u = num / div;
        double xs = p1.x() + u * (p2.x() - p1.x());
        double ys = p1.y() + u * (p2.y() - p1.y());
        const geo::Coordinate coord(xs, ys);

        const geo::Area a1(p1, p2);
        const geo::Area a2(p3, p4);

        const bool a1b = a1.inArea(coord);
        const bool a2b = a2.inArea(coord);

        if (_method == Method::OnPath) {
            _intersectionPoints.push_back(coord);
        } else if (a1b && a2b) { // Test if it positivly fit's within a area
            _intersectionPoints.push_back(coord);
        } else if (
                (p1.x() == p2.x() && ys >= a1.minP().y() && ys <= a1.maxP().y() && a2b) ||
                // when we deal with horizontal or vertical lines, inArea might not
                (p3.x() == p4.x() && ys >= a2.minP().y() && ys <= a2.maxP().y() && a1b) ||
                // give a positive result, this conditions will confirm without using tolerance
                (p1.y() == p2.y() && xs >= a1.minP().x() && xs <= a1.maxP().x() && a2b) ||
                (p3.y() == p4.y() && xs >= a2.minP().x() && xs <= a2.maxP().x() && a1b)
                ) {
            _intersectionPoints.push_back(coord);
        }
    }
}

void Intersect::geovisit(const geo::Vector& v, const geo::Arc& arc) {
    auto &&coords = maths::Intersection::LineQuad(v.equation(), arc.equation());

    if (_method == Method::OnPath) {
        _intersectionPoints.reserve(_intersectionPoints.size() + coords.size());
        _intersectionPoints.insert(coords.end(), coords.begin(), coords.end());
    }
    else {
        for (auto &point : coords) {
            double a = (point - arc.center()).angle();
            if (arc.isAngleBetween(a) &&
                v.nearestPointOnEntity(point).distanceTo(point) < LCTOLERANCE) {
                _intersectionPoints.push_back(point);
            }
        }
    }
}

void Intersect::geovisit(const geo::Vector& v, const geo::Ellipse& ellipse) {
    auto &&coords = maths::Intersection::LineQuad(v.equation(), ellipse.equation());

    if (_method == Method::OnPath) {
        _intersectionPoints.reserve(_intersectionPoints.size() + coords.size());
        _intersectionPoints.insert(coords.end(), coords.begin(), coords.end());
    }
    else {
        for (auto &point : coords) {
            double a = (point - ellipse.center()).angle();
            if (ellipse.isAngleBetween(a) &&
                v.nearestPointOnEntity(point).distanceTo(point) < LCTOLERANCE) {
                _intersectionPoints.push_back(point);
            }
        }
    }
}

void Intersect::geovisit(const geo::Arc& arc1, const geo::Arc& arc2) {
    auto &&coords = maths::Intersection::QuadQuad(arc1.equation(), arc2.equation());

    if (_method == Method::OnPath) {
        _intersectionPoints.reserve(_intersectionPoints.size() + coords.size());
        _intersectionPoints.insert(coords.end(), coords.begin(), coords.end());
    }
    else {
        for (auto &point : coords) {
            double a1 = (point - arc1.center()).angle();
            double a2 = (point - arc2.center()).angle();
            if (arc1.isAngleBetween(a1) &&
                arc2.isAngleBetween(a2)) {
                _intersectionPoints.push_back(point);
            }
        }
    }
}

void Intersect::geovisit(const geo::Arc& arc, const geo::Ellipse& ellipse) {
    auto &&coords = maths::Intersection::QuadQuad(arc.equation(), ellipse.equation());

    if (_method == Method::OnPath) {
        _intersectionPoints.reserve(_intersectionPoints.size() + coords.size());
        _intersectionPoints.insert(coords.end(), coords.begin(), coords.end());
    }
    else {
        for (auto &point : coords) {
            double a1 = (point - arc.center()).angle();
            double a2 = (point - ellipse.center()).angle();
            if (arc.isAngleBetween(a1) &&
                ellipse.isAngleBetween(a2)) {
                _intersectionPoints.push_back(point);
            }
        }
    }
}

void Intersect::geovisit(const geo::Ellipse& ellipse1, const geo::Ellipse& ellipse2) {
    auto &&coords = maths::Intersection::QuadQuad(ellipse1.equation(), ellipse2.equation());

    if (_method == Method::OnPath) {
        _intersectionPoints.reserve(_intersectionPoints.size() + coords.size());
        _intersectionPoints.insert(coords.end(), coords.begin(), coords.end());
    }
    else {
        for (auto &point : coords) {
            double a1 = (point - ellipse1.center()).angle();
            double a2 = (point - ellipse2.center()).angle();
            if (ellipse1.isAngleBetween(a1) &&
                ellipse2.isAngleBetween(a2)) {
                _intersectionPoints.push_back(point);
            }
        }
    }
}
/***
 *    ~|~ _ _|_ _  _ _ _  __|_|\/| _  _
 *    _|_| | | (/_| _\(/_(_ | |  |(_|| |\/
 *                                      /
 */
IntersectMany::IntersectMany(std::vector<entity::CADEntity_CSPtr> entities, Intersect::Method method, double tolerance) :
        _entities(std::move(entities)),
        _method(method),
        _tolerance(tolerance) {

}

std::vector<lc::geo::Coordinate> IntersectMany::result() const {
    Intersect intersect(_method, _tolerance);
    if (_entities.size() > 1) {
        for (size_t outer = 0; outer < (_entities.size() - 1); outer++) {
            for (size_t inner = outer + 1; inner < _entities.size(); inner++) {
                visitorDispatcher<bool, lc::GeoEntityVisitor>(intersect, *_entities.at(outer).get(), *_entities.at(inner).get());
            }
        }
    }
    return intersect.result();
}

/***
 *    ~|~ _ _|_ _  _ _ _  __|_/\  _  _ . _  __|_/~\_|_|_  _  _ _
 *    _|_| | | (/_| _\(/_(_ |/~~\(_|(_||| |_\ | \_/ | | |(/_| _\
 *                                _|                            
 */
IntersectAgainstOthers::IntersectAgainstOthers(std::vector<entity::CADEntity_CSPtr> entities,
                                               std::vector<entity::CADEntity_CSPtr> others, Intersect::Method method,
                                               double tolerance) :
        _entities(std::move(entities)),
        _others(std::move(others)),
        _method(method),
        _tolerance(tolerance) {
}

std::vector<lc::geo::Coordinate> IntersectAgainstOthers::result() const {
    Intersect intersect(_method, _tolerance);
    /*FIXME unused
    for (auto &other : _others) {
        for (auto &entity : _entities) {
               visitorDispatcher<bool, lc::GeoEntityVisitor>(intersect, *other.get(), *entity.get());
        }
    }
    */
    std::cerr << __PRETTY_FUNCTION__ << " requires implementation" << std::endl;

    return intersect.result();
}

