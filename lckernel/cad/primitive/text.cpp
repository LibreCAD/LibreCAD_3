#include "text.h"

using namespace lc;

Text::Text(const geo::Coordinate& insertion_point, double height, QString text_value, const Layer_CSPtr layer) : CADEntity(layer), geo::Text(insertion_point, height, text_value) {

}

Text::Text(const geo::Coordinate& insertion_point, double height, QString text_value, const Layer_CSPtr layer, const std::list<MetaType_CSPtr >& metaTypes) : CADEntity(layer, metaTypes),  geo::Text(insertion_point, height, text_value) {
}

CADEntity_CSPtr Text::move(const geo::Coordinate& offset) const {

}

CADEntity_CSPtr Text::copy(const geo::Coordinate& offset) const {

}

CADEntity_CSPtr Text::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {

}

CADEntity_CSPtr Text::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {

}
