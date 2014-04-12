#include "text.h"

using namespace lc;

Text::Text(const geo::Coordinate& insertion_point, double height, QString text_value ) : CADEntity(), geo::Text(insertion_point, height, text_value) {

}

Text::Text(const geo::Coordinate& insertion_point, double height, QString text_value, const QList<shared_ptr<const lc::MetaType> >& metaTypes) : CADEntity(metaTypes),  geo::Text(insertion_point, height, text_value) {
}

shared_ptr<const lc::CADEntity> Text::move(const geo::Coordinate& offset) const {

}

shared_ptr<const lc::CADEntity> Text::copy(const geo::Coordinate& offset) const {

}

shared_ptr<const lc::CADEntity> Text::rotate(const geo::Coordinate &rotation_center, const double& rotation_angle, const bool with_same_id) const {

}

shared_ptr<const lc::CADEntity> Text::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {

}
