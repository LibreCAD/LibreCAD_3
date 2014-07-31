#include "entitytest.h"
#include <iostream>
entitytest::entitytest() {

}

std::vector<Line_CSPtr> entitytest::LineMove(geo::Coordinate & offset) {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push();
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(-100., -100.), layer)).push();
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., -100.), layer)).push();
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(-100., 100.), layer)).push();
    builder->move(offset);
    builder->execute();

    auto _entities = _document->entityContainer().allEntities();

    std::vector<Line_CSPtr> lines;
    for(auto i : _entities) {
        lines.push_back(std::dynamic_pointer_cast<const lc::Line>(i));
    }

    return lines;
}

std::vector<Line_CSPtr> entitytest::LineCopy(geo::Coordinate & offset) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);

    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., -50.), lc::geo::Coordinate(100., 100.), layer)).push();
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., -50.), lc::geo::Coordinate(-100., -100.), layer)).push();
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 50.), lc::geo::Coordinate(100., -100.), layer)).push();
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 50.), lc::geo::Coordinate(-100., 100.), layer)).push();
    builder->copy(offset);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    std::vector<Line_CSPtr> lines;
    for(auto i : _entities) {
        lines.push_back(std::dynamic_pointer_cast<const lc::Line>(i));
    }

    return lines;
}

std::vector<Line_CSPtr> entitytest::LineRotate(geo::Coordinate & center, double angle) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);

    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., -50.), lc::geo::Coordinate(100., 100.), layer)).push();
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., -50.), lc::geo::Coordinate(-100., -100.), layer)).push();
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 50.), lc::geo::Coordinate(100., -100.), layer)).push();
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 50.), lc::geo::Coordinate(-100., 100.), layer)).push();
    builder->rotate(center,angle);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    std::vector<Line_CSPtr> lines;
    for(auto i : _entities) {
        lines.push_back(std::dynamic_pointer_cast<const lc::Line>(i));
    }

    return lines;
}

std::vector<Line_CSPtr> entitytest::LineScale(geo::Coordinate & center, geo::Coordinate & ratio) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);

    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., -50.), lc::geo::Coordinate(100., 100.), layer)).push();
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., -50.), lc::geo::Coordinate(-100., -100.), layer)).push();
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 50.), lc::geo::Coordinate(100., -100.), layer)).push();
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 50.), lc::geo::Coordinate(-100., 100.), layer)).push();
    builder->scale(center, ratio);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    std::vector<Line_CSPtr> lines;
    for(auto i : _entities) {
        lines.push_back(std::dynamic_pointer_cast<const lc::Line>(i));
    }

    return lines;
}

std::vector<Circle_CSPtr> entitytest::CircleMove(geo::Coordinate & offset) {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);

    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();

    std::vector<Circle_CSPtr> circles;
    for(auto i : _entities) {
        circles.push_back(std::dynamic_pointer_cast<const lc::Circle>(i));
    }

    return circles;
}

std::vector<Circle_CSPtr> entitytest::CircleCopy(geo::Coordinate & offset) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);

    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();

    builder->execute();
    auto _entities = _document->entityContainer().allEntities();
    std::vector<Circle_CSPtr> circles;
    for(auto i : _entities) {
        circles.push_back(std::dynamic_pointer_cast<const lc::Circle>(i));
    }

    return circles;
}

std::vector<Circle_CSPtr> entitytest::CircleRotate(geo::Coordinate & center, double angle) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);

    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();

    builder->execute();
    auto _entities = _document->entityContainer().allEntities();
    std::vector<Circle_CSPtr> circles;
    for(auto i : _entities) {
        circles.push_back(std::dynamic_pointer_cast<const lc::Circle>(i));
    }

    return circles;
}

std::vector<Circle_CSPtr> entitytest::CircleScale(geo::Coordinate & center, geo::Coordinate & ratio) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);

    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();

    builder->execute();
    auto _entities = _document->entityContainer().allEntities();
    std::vector<Circle_CSPtr> circles;
    for(auto i : _entities) {
        circles.push_back(std::dynamic_pointer_cast<const lc::Circle>(i));
    }

    return circles;
}

std::vector<Arc_CSPtr> entitytest::ArcMove(geo::Coordinate & offset) {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);

    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();
    std::vector<Arc_CSPtr> arcs;
    for(auto i : _entities) {
        arcs.push_back(std::dynamic_pointer_cast<const lc::Arc>(i));
    }

    return arcs;
}

std::vector<Arc_CSPtr> entitytest::ArcCopy(geo::Coordinate & offset) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);

    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();
    std::vector<Arc_CSPtr> arcs;
    for(auto i : _entities) {
        arcs.push_back(std::dynamic_pointer_cast<const lc::Arc>(i));
    }

    return arcs;
}

std::vector<Arc_CSPtr> entitytest::ArcRotate(geo::Coordinate & center, double angle) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);

    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();
    std::vector<Arc_CSPtr> arcs;
    for(auto i : _entities) {
        arcs.push_back(std::dynamic_pointer_cast<const lc::Arc>(i));
    }

    return arcs;
}

std::vector<Arc_CSPtr> entitytest::ArcScale(geo::Coordinate & center, geo::Coordinate & ratio) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);

    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925  , layer)).push();
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();
    std::vector<Arc_CSPtr> arcs;
    for(auto i : _entities) {
        arcs.push_back(std::dynamic_pointer_cast<const lc::Arc>(i));
    }

    return arcs;
}


std::vector<Ellipse_CSPtr> entitytest::EllipseMove(geo::Coordinate & offset) {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);

    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();
    std::vector<Ellipse_CSPtr> ellipses;
    for(auto i : _entities) {
        ellipses.push_back(std::dynamic_pointer_cast<const lc::Ellipse>(i));
    }

    return ellipses;
}

std::vector<Ellipse_CSPtr> entitytest::EllipseCopy(geo::Coordinate & offset) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);

    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();
    std::vector<Ellipse_CSPtr> ellipses;
    for(auto i : _entities) {
        ellipses.push_back(std::dynamic_pointer_cast<const lc::Ellipse>(i));
    }

    return ellipses;
}

std::vector<Ellipse_CSPtr> entitytest::EllipseRotate(geo::Coordinate & center, double angle) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);

    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->rotate(center, angle);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();
    std::vector<Ellipse_CSPtr> ellipses;
    for(auto i : _entities) {
        ellipses.push_back(std::dynamic_pointer_cast<const lc::Ellipse>(i));
    }

    return ellipses;
}

std::vector<Ellipse_CSPtr> entitytest::EllipseScale(geo::Coordinate & center, geo::Coordinate & ratio) {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);

    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->scale(center, ratio);
    builder->execute();
    auto _entities = _document->entityContainer().allEntities();
    std::vector<Ellipse_CSPtr> ellipses;
    for(auto i : _entities) {
        ellipses.push_back(std::dynamic_pointer_cast<const lc::Ellipse>(i));
    }

    return ellipses;
}
