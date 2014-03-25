#include "text.h"

using namespace lc;

Text::Text(const geo::Coordinate& insertion_point, double height, QString text_value ) : CADEntity(), geo::Text(insertion_point, height, text_value) {

}

Text::Text(const geo::Coordinate& insertion_point, double height, QString text_value, const QList<shared_ptr<const lc::MetaType> >& metaTypes) : CADEntity(metaTypes),  geo::Text(insertion_point, height, text_value) {
}

shared_ptr<const lc::CADEntity> Text::move(const geo::Coordinate &offset) const {
    Text * tex = new lc::Text(this->insertion_point() + offset, this->height(), this->text_value());
    tex->setID(this->id());
    shared_ptr<const lc::Text> newText = shared_ptr<const lc::Text>(tex);
    return newText;
}

shared_ptr<const lc::CADEntity> Text::rotate(const geo::Coordinate &rotation_point, const double& angle) const {
    //return;
}
