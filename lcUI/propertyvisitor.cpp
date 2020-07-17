#include "propertyvisitor.h"

using namespace lc::ui;

void PropertyVisitor::visit(entity::Line_CSPtr) {
    entityInfo = "Line";
}

void PropertyVisitor::visit(entity::Point_CSPtr) {
    entityInfo = "Point";
}

void PropertyVisitor::visit(entity::Circle_CSPtr entity) {
    lc::builder::CircleBuilder circleBuilder;
    circleBuilder.copy(entity);

    if (propertyKey == "entity" + std::to_string(entity->id()) + "_radius") {
        double circleRadius = propertiesTable[propertyKey].get<double>();
        circleBuilder.setRadius(circleRadius);
    }
    else if (propertyKey == "entity" + std::to_string(entity->id()) + "_center") {
        lc::geo::Coordinate circleCenter = propertiesTable[propertyKey].get<lc::geo::Coordinate>();
        circleBuilder.setCenter(circleCenter);
    }

    lc::entity::Circle_CSPtr circleEntity = circleBuilder.build();
    entityBuilder->appendEntity(circleEntity);
    entityBuilder->execute();
}

void PropertyVisitor::visit(entity::Arc_CSPtr) {
    entityInfo = "Arc";
}

void PropertyVisitor::visit(entity::Ellipse_CSPtr) {
    entityInfo = "Ellipse";
}

void PropertyVisitor::visit(entity::Text_CSPtr) {
    entityInfo = "Text";
}

void PropertyVisitor::visit(entity::Spline_CSPtr) {
    entityInfo = "Spline";
}

void PropertyVisitor::visit(entity::DimAligned_CSPtr) {
    entityInfo = "DimAligned";
}

void PropertyVisitor::visit(entity::DimAngular_CSPtr) {
    entityInfo = "DimAngular";
}

void PropertyVisitor::visit(entity::DimDiametric_CSPtr) {
    entityInfo = "DimDiametric";
}

void PropertyVisitor::visit(entity::DimLinear_CSPtr) {
    entityInfo = "DimLinear";
}

void PropertyVisitor::visit(entity::DimRadial_CSPtr) {
    entityInfo = "DimRadial";
}

void PropertyVisitor::visit(entity::LWPolyline_CSPtr) {
    entityInfo = "LWPolyline";
}

void PropertyVisitor::visit(entity::Image_CSPtr) {
    entityInfo = "Image";
}

void PropertyVisitor::visit(entity::Hatch_CSPtr) {
    entityInfo = "Hatch";
}

void PropertyVisitor::setPropertyKey(const std::string& key) {
    propertyKey = key;
}

void PropertyVisitor::setPropertyTable(kaguya::LuaRef propTable) {
    propertiesTable = propTable;
}

void PropertyVisitor::setEntityBuilder(std::shared_ptr<lc::operation::EntityBuilder> entBuilder) {
    entityBuilder = entBuilder;
}
