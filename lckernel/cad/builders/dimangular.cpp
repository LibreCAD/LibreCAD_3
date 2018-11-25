#include "dimangular.h"
#include <cad/primitive/dimangular.h>

const lc::geo::Coordinate& lc::builder::DimAngularBuilder::defLine11() const {
    return _defLine11;
}

void lc::builder::DimAngularBuilder::setDefLine11(const lc::geo::Coordinate& defLine11) {
    _defLine11 = defLine11;
}

const lc::geo::Coordinate& lc::builder::DimAngularBuilder::defLine12() const {
    return _defLine12;
}

void lc::builder::DimAngularBuilder::setDefLine12(const lc::geo::Coordinate& defLine12) {
    _defLine12 = defLine12;
}

const lc::geo::Coordinate& lc::builder::DimAngularBuilder::defLine21() const {
    return _defLine21;
}

void lc::builder::DimAngularBuilder::setDefLine21(const lc::geo::Coordinate& defLine21) {
    _defLine21 = defLine21;
}

const lc::geo::Coordinate& lc::builder::DimAngularBuilder::defLine22() const {
    return _defLine22;
}

void lc::builder::DimAngularBuilder::setDefLine22(const lc::geo::Coordinate& defLine22) {
    _defLine22 = defLine22;
}

void lc::builder::DimAngularBuilder::dimAuto(const lc::geo::Coordinate& center,
                                             const lc::geo::Coordinate& p1,
                                             const lc::geo::Coordinate& p2) {
    _definitionPoint = center;
    _middleOfText = p1.mid(p2);
    _defLine11 = center;
    _defLine12 = p1;
    _defLine21 = center;
    _defLine22 = p2;
}

lc::entity::DimAngular_CSPtr lc::builder::DimAngularBuilder::build() {
    return lc::entity::DimAngular_CSPtr(new lc::entity::DimAngular(*this));
}
