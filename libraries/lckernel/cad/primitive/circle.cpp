#include "cad/primitive/circle.h"

using namespace lc;

Circle::Circle(const geo::Coordinate& center, double radius) : CADEntity(), geo::Circle(center, radius) {

}

Circle::Circle(const geo::Coordinate& center, double radius, const QList<MetaTypePtr>& metaTypes) : CADEntity(metaTypes),  geo::Circle(center, radius) {
}

