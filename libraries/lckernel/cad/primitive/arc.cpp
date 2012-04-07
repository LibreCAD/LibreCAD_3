#include "arc.h"

using namespace lc;

Arc::Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle) : CADEntity(), geo::Arc(center, radius, startAngle, endAngle) {

}

Arc::Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, const QList<MetaTypePtr>& metaTypes) : CADEntity(metaTypes),  geo::Arc(center, radius, startAngle, endAngle) {
}

