#include "entitytest.h"
#include <iostream>
entitytest::entitytest() {

}

Line_CSPtr entitytest::LineMove(geo::Coordinate & offset) {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push().move(offset);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto line = std::dynamic_pointer_cast<const lc::Line>(_entities.at(0));

    return line;
}

Line_CSPtr entitytest::LineCopy(geo::Coordinate & offset) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push().copy(offset);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto line = std::dynamic_pointer_cast<const lc::Line>(_entities.at(0));
    return line;
}

Line_CSPtr entitytest::LineRotate(geo::Coordinate & center, double angle) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push().rotate(center, angle);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto line = std::dynamic_pointer_cast<const lc::Line>(_entities.at(0));
    return line;
}

Line_CSPtr entitytest::LineScale(geo::Coordinate & center, geo::Coordinate & ratio) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push().scale(center, ratio);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto line = std::dynamic_pointer_cast<const lc::Line>(_entities.at(0));
    return line;
}

Circle_CSPtr entitytest::CircleMove(geo::Coordinate & offset) {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push().move(offset);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto circle = std::dynamic_pointer_cast<const lc::Circle>(_entities.at(0));
    return circle;
}

Circle_CSPtr entitytest::CircleCopy(geo::Coordinate & offset) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push().copy(offset);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto circle = std::dynamic_pointer_cast<const lc::Circle>(_entities.at(0));
    return circle;
}

Circle_CSPtr entitytest::CircleRotate(geo::Coordinate & center, double angle) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push().rotate(center, angle);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto circle = std::dynamic_pointer_cast<const lc::Circle>(_entities.at(0));
    return circle;
}

Circle_CSPtr entitytest::CircleScale(geo::Coordinate & center, geo::Coordinate & ratio) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push().scale(center, ratio);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto circle = std::dynamic_pointer_cast<const lc::Circle>(_entities.at(0));
    return circle;
}

Arc_CSPtr entitytest::ArcMove(geo::Coordinate & offset) {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push().move(offset);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto arc = std::dynamic_pointer_cast<const lc::Arc>(_entities.at(0));
    return arc;
}

Arc_CSPtr entitytest::ArcCopy(geo::Coordinate & offset) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push().copy(offset);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto arc = std::dynamic_pointer_cast<const lc::Arc>(_entities.at(0));
    return arc;
}

Arc_CSPtr entitytest::ArcRotate(geo::Coordinate & center, double angle) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push().rotate(center, angle);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto arc = std::dynamic_pointer_cast<const lc::Arc>(_entities.at(0));
    return arc;
}

Arc_CSPtr entitytest::ArcScale(geo::Coordinate & center, geo::Coordinate & ratio) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push().scale(center, ratio);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto arc = std::dynamic_pointer_cast<const lc::Arc>(_entities.at(0));
    return arc;
}


Ellipse_CSPtr entitytest::EllipseMove(geo::Coordinate & offset) {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push().move(offset);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto ellipse = std::dynamic_pointer_cast<const lc::Ellipse>(_entities.at(0));
    return ellipse;
}

Ellipse_CSPtr entitytest::EllipseCopy(geo::Coordinate & offset) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push().copy(offset);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto ellipse = std::dynamic_pointer_cast<const lc::Ellipse>(_entities.at(0));
    return ellipse;
}

Ellipse_CSPtr entitytest::EllipseRotate(geo::Coordinate & center, double angle) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push().rotate(center, angle);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto ellipse = std::dynamic_pointer_cast<const lc::Ellipse>(_entities.at(0));
    return ellipse;
}

Ellipse_CSPtr entitytest::EllipseScale(geo::Coordinate & center, geo::Coordinate & ratio) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push().scale(center, ratio);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    auto ellipse = std::dynamic_pointer_cast<const lc::Ellipse>(_entities.at(0));
    return ellipse;
}
