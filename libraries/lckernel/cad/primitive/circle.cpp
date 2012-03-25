#include "circle.h"

using namespace lc;

Circle::Circle(const geo::Coordinate& center, double radius) : CADEntity(), geo::Sphere(center, radius) {
}
Circle::Circle(const geo::Coordinate& center, double radius, METAINFO_TYPELIST metaTypes) : CADEntity(metaTypes),  geo::Sphere(center, radius) {
}


bool Circle::inArea(const geo::Area& area) const {
    return true;
}
