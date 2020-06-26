#include "entitypickervisitor.h"

using namespace lc::ui::api;

void EntityPickerVisitor::visit(entity::Line_CSPtr) {
    entityInfo = "Line";
}

void EntityPickerVisitor::visit(entity::Point_CSPtr) {
    entityInfo = "Point";
}

void EntityPickerVisitor::visit(entity::Circle_CSPtr){
    entityInfo = "Circle";
}

void EntityPickerVisitor::visit(entity::Arc_CSPtr){
    entityInfo = "Arc";
}

void EntityPickerVisitor::visit(entity::Ellipse_CSPtr){
    entityInfo = "Ellipse";
}

void EntityPickerVisitor::visit(entity::Text_CSPtr){
    entityInfo = "Text";
}

void EntityPickerVisitor::visit(entity::Spline_CSPtr){
    entityInfo = "Spline";
}

void EntityPickerVisitor::visit(entity::DimAligned_CSPtr){
    entityInfo = "DimAligned";
}

void EntityPickerVisitor::visit(entity::DimAngular_CSPtr){
    entityInfo = "DimAngular";
}

void EntityPickerVisitor::visit(entity::DimDiametric_CSPtr){
    entityInfo = "DimDiametric";
}

void EntityPickerVisitor::visit(entity::DimLinear_CSPtr){
    entityInfo = "DimLinear";
}

void EntityPickerVisitor::visit(entity::DimRadial_CSPtr){
    entityInfo = "DimRadial";
}

void EntityPickerVisitor::visit(entity::LWPolyline_CSPtr){
    entityInfo = "LWPolyline";
}

void EntityPickerVisitor::visit(entity::Image_CSPtr){
    entityInfo = "Image";
}

void EntityPickerVisitor::visit(entity::Hatch_CSPtr){
    entityInfo = "Hatch";
}

std::string EntityPickerVisitor::getEntityInformation() const {
    return entityInfo;
}
