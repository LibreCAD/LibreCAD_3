#include "entitynamevisitor.h"

using namespace lc::ui::api;

void EntityNameVisitor::visit(entity::Line_CSPtr) {
    entityInfo = "Line";
}

void EntityNameVisitor::visit(entity::Point_CSPtr) {
    entityInfo = "Point";
}

void EntityNameVisitor::visit(entity::Circle_CSPtr){
    entityInfo = "Circle";
}

void EntityNameVisitor::visit(entity::Arc_CSPtr){
    entityInfo = "Arc";
}

void EntityNameVisitor::visit(entity::Ellipse_CSPtr){
    entityInfo = "Ellipse";
}

void EntityNameVisitor::visit(entity::Text_CSPtr){
    entityInfo = "Text";
}

void EntityNameVisitor::visit(entity::Spline_CSPtr){
    entityInfo = "Spline";
}

void EntityNameVisitor::visit(entity::DimAligned_CSPtr){
    entityInfo = "DimAligned";
}

void EntityNameVisitor::visit(entity::DimAngular_CSPtr){
    entityInfo = "DimAngular";
}

void EntityNameVisitor::visit(entity::DimDiametric_CSPtr){
    entityInfo = "DimDiametric";
}

void EntityNameVisitor::visit(entity::DimLinear_CSPtr){
    entityInfo = "DimLinear";
}

void EntityNameVisitor::visit(entity::DimRadial_CSPtr){
    entityInfo = "DimRadial";
}

void EntityNameVisitor::visit(entity::LWPolyline_CSPtr){
    entityInfo = "LWPolyline";
}

void EntityNameVisitor::visit(entity::Image_CSPtr){
    entityInfo = "Image";
}

void EntityNameVisitor::visit(entity::Hatch_CSPtr){
    entityInfo = "Hatch";
}

std::string EntityNameVisitor::getEntityInformation() const {
    return entityInfo;
}
