
#include "cad/primitive/dimaligned.h"

using namespace lc;

dimAligned::dimAligned(const Dimension dimension,
        const geo::Coordinate& extension_point1,
                     const geo::Coordinate& extension_point2,
                      const Layer_CSPtr layer): CADEntity(layer), geo::dimAligned(dimension, extension_point1, extension_point2) {

}

dimAligned::dimAligned(const Dimension dimension,
        const geo::Coordinate& extension_point1,
                     const geo::Coordinate& extension_point2,
                      const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes): CADEntity(layer), geo::dimAligned(dimension, extension_point1, extension_point2) {

}


//CADEntity_CSPtr dimAligned::move(const geo::Coordinate& offset) const {
//    auto newdimAligned = std::make_shared<dimAligned>(this->definition_point() + offset, this->middle_of_text() + offset,
//                                                    this->valign(), this->halign(),
//                                                    this->lineSpacingStyle(), this->lineSpacingFactor(),
//                                                    this->text_value(), this->style(), this->angle(), layer());
//    newdimAligned->setID(this->id());
//    return newdimAligned;
//}

//CADEntity_CSPtr dimAligned::copy(const geo::Coordinate& offset) const {
//    auto newdimAligned = std::make_shared<dimAligned>(this->definition_point() + offset, this->middle_of_text() + offset,
//                                                    this->valign(), this->halign(),
//                                                    this->lineSpacingStyle(), this->lineSpacingFactor(),
//                                                    this->text_value(), this->style(), this->angle(), layer());
//    return newdimAligned;
//}

//CADEntity_CSPtr dimAligned::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
//    auto newdimAligned = std::make_shared<dimAligned>(this->definition_point().rotate(rotation_center, rotation_angle), this->middle_of_text(),
//                                                    this->valign(), this->halign(),
//                                                    this->lineSpacingStyle(), this->lineSpacingFactor(),
//                                                    this->text_value(), this->style(), this->angle(), layer());
//    newdimAligned->setID(this->id());
//    return newdimAligned;
//}

//CADEntity_CSPtr dimAligned::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
//    auto newdimAligned = std::make_shared<dimAligned>(this->definition_point().scale(scale_center, scale_factor), this->middle_of_text().scale(scale_center, scale_factor),
//                                                    this->valign(), this->halign(), this->lineSpacingStyle(), this->lineSpacingFactor(),
//                                                    this->text_value(), this->style(), this->angle(), layer());
//    newdimAligned->setID(this->id());
//    return newdimAligned;
//}

const geo::Area dimAligned::boundingBox() const {
    // TODO create bounding box for dimAligned
    return geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(0., 0.));
}
