#include "text.h"

using namespace lc;

Text::Text(const geo::Coordinate& insertion_point, double height, QString text_value ) : CADEntity(), geo::Text(insertion_point, height, text_value) {

}

Text::Text(const geo::Coordinate& insertion_point, double height, QString text_value, const QList<shared_ptr<const lc::MetaType> >& metaTypes) : CADEntity(metaTypes),  geo::Text(insertion_point, height, text_value) {
}
