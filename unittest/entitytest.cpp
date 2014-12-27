#include "entitytest.h"
#include <iostream>
entitytest::entitytest() {

}

std::vector<Line_CSPtr> entitytest::LineMove() {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset1(10.0, 10.0);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push();
    builder->move(_offset1);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset2(20.0, 20.0);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(110., 0.), lc::geo::Coordinate(100., 110.), layer)).push();;
    builder->move(_offset2);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset3(-20.0, 20.0);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(550., 60.), lc::geo::Coordinate(90., -100.), layer)).push();;
    builder->move(_offset3);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset4(-20.0, -20.0);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(200., 750.), lc::geo::Coordinate(-100., 100.), layer)).push();;
    builder->move(_offset4);
    builder->execute();

    auto _entities = _document->entityContainer().asVector();

    std::vector<Line_CSPtr> lines;

    for (auto i : _entities) {
        lines.push_back(std::dynamic_pointer_cast<const lc::Line>(i));
    }

    return lines;
}


std::vector<Line_CSPtr> entitytest::LineCopy() {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset1(10.0, 10.0);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push();
    builder->copy(_offset1);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset2(20.0, 20.0);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(110., 0.), lc::geo::Coordinate(100., 110.), layer)).push();;
    builder->copy(_offset2);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset3(-20.0, 20.0);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(550., 60.), lc::geo::Coordinate(90., -100.), layer)).push();;
    builder->copy(_offset3);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset4(-20.0, -20.0);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(200., 750.), lc::geo::Coordinate(-100., 100.), layer)).push();;
    builder->copy(_offset4);
    builder->execute();

    auto _entities = _document->entityContainer().asVector();

    std::vector<Line_CSPtr> lines;

    for (auto i : _entities) {
        lines.push_back(std::dynamic_pointer_cast<const lc::Line>(i));
    }

    return lines;
}

std::vector<Line_CSPtr> entitytest::LineRotate() {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center1(0.0, 0.0);
    double angle1 = 30;
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push();
    builder->rotate(_center1, angle1);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center2(-20.0, -20.0);
    double angle2 = 60.;
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push();;
    builder->rotate(_center2, angle2);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center3(10.0, 43.0);
    double angle3 = 90;
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push();;
    builder->rotate(_center3, angle3);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center4(55.0, 20.0);
    double angle4 = 180;
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push();;
    builder->rotate(_center4, angle4);
    builder->execute();

    auto _entities = _document->entityContainer().asVector();

    std::vector<Line_CSPtr> lines;

    for (auto i : _entities) {
        lines.push_back(std::dynamic_pointer_cast<const lc::Line>(i));
    }

    return lines;
}

std::vector<Line_CSPtr> entitytest::LineScale() {
    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();

    auto _document = new lc::DocumentImpl(_storageManager);

    auto layer = _storageManager->layerByName("0");
    auto builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center1(10.0, 10.0);
    lc::geo::Coordinate _ratio1(5.0, 5.0);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push();
    builder->scale(_center1, _ratio1);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center2(-20.0, -20.0);
    lc::geo::Coordinate _ratio2(2.0, 3.0);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push();;
    builder->scale(_center2, _ratio2);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center3(55.0, 72.0);
    lc::geo::Coordinate _ratio3(2.0, 6.0);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push();;
    builder->scale(_center3, _ratio3);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center4(10.0, 21.0);
    lc::geo::Coordinate _ratio4(20.0, 20.0);
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), layer)).push();;
    builder->scale(_center4, _ratio4);
    builder->execute();

    auto _entities = _document->entityContainer().asVector();

    std::vector<Line_CSPtr> lines;

    for (auto i : _entities) {
        lines.push_back(std::dynamic_pointer_cast<const lc::Line>(i));
    }

    return lines;
}

std::vector<Circle_CSPtr> entitytest::CircleMove() {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();
    auto _document = new lc::DocumentImpl(_storageManager);
    auto layer = _storageManager->layerByName("0");

    auto builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset1(0.0, 0.0);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->move(_offset1);
    builder->execute();


    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset2(20.0, -20.0);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->move(_offset2);
    builder->execute();


    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset3(-20.0, -30.0);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->move(_offset3);
    builder->execute();


    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset4(-20.0, 20.0);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->move(_offset4);
    builder->execute();

    auto _entities = _document->entityContainer().asVector();

    std::vector<Circle_CSPtr> circles;

    for (auto i : _entities) {
        circles.push_back(std::dynamic_pointer_cast<const lc::Circle>(i));
    }

    return circles;
}

std::vector<Circle_CSPtr> entitytest::CircleCopy() {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();
    auto _document = new lc::DocumentImpl(_storageManager);
    auto layer = _storageManager->layerByName("0");

    auto builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset1(0.0, 0.0);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(10., 0.), 50., layer)).push();
    builder->copy(_offset1);
    builder->execute();


    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset2(20.0, -20.0);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 20.), 50., layer)).push();
    builder->copy(_offset2);
    builder->execute();


    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset3(-20.0, -30.0);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(22., -50.), 50., layer)).push();
    builder->copy(_offset3);
    builder->execute();


    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset4(-20.0, 20.0);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(-555., 1000.), 50., layer)).push();
    builder->copy(_offset4);
    builder->execute();


    auto _entities = _document->entityContainer().asVector();

    std::vector<Circle_CSPtr> circles;

    for (auto i : _entities) {
        circles.push_back(std::dynamic_pointer_cast<const lc::Circle>(i));
    }

    return circles;
}

std::vector<Circle_CSPtr> entitytest::CircleRotate() {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();
    auto _document = new lc::DocumentImpl(_storageManager);
    auto layer = _storageManager->layerByName("0");

    auto builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center1(0.0, 0.0);
    double angle1 = -45.;
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->rotate(_center1, angle1);
    builder->execute();


    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center2(-20.0, -0.0);
    double angle2 = 30.;
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->rotate(_center2, angle2);
    builder->execute();


    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center3(60.0, 30.0);
    double angle3 = -100.;
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->rotate(_center3, angle3);
    builder->execute();


    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center4(20.0, 20.0);
    double angle4 = 180.;
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->rotate(_center4, angle4);
    builder->execute();

    auto _entities = _document->entityContainer().asVector();

    std::vector<Circle_CSPtr> circles;

    for (auto i : _entities) {
        circles.push_back(std::dynamic_pointer_cast<const lc::Circle>(i));
    }

    return circles;
}

std::vector<Circle_CSPtr> entitytest::CircleScale() {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();
    auto _document = new lc::DocumentImpl(_storageManager);
    auto layer = _storageManager->layerByName("0");

    auto builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center1(0.0, 0.0);
    lc::geo::Coordinate _ratio1(5, 2);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->scale(_center1, _ratio1);
    builder->execute();


    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center2(100.0, 100.0);
    lc::geo::Coordinate _ratio2(2, 2);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->scale(_center2, _ratio2);
    builder->execute();


    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center3(-100.0, -100.0);
    lc::geo::Coordinate _ratio3(1, 7);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->scale(_center3, _ratio3);
    builder->execute();


    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center4(73.0, 20.0);
    lc::geo::Coordinate _ratio4(4, 10);
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0., 0.), 50., layer)).push();
    builder->scale(_center4, _ratio4);
    builder->execute();

    auto _entities = _document->entityContainer().asVector();

    std::vector<Circle_CSPtr> circles;

    for (auto i : _entities) {
        circles.push_back(std::dynamic_pointer_cast<const lc::Circle>(i));
    }

    return circles;
}

std::vector<Arc_CSPtr> entitytest::ArcMove() {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();
    auto _document = new lc::DocumentImpl(_storageManager);
    auto layer = _storageManager->layerByName("0");

    auto builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset1(20., 10.);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->move(_offset1);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset2(33., 21.);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->move(_offset2);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset3(75., 75.);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->move(_offset3);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset4(3000., 1110.);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->move(_offset4);
    builder->execute();

    auto _entities = _document->entityContainer().asVector();
    std::vector<Arc_CSPtr> arcs;

    for (auto i : _entities) {
        arcs.push_back(std::dynamic_pointer_cast<const lc::Arc>(i));
    }

    return arcs;
}

std::vector<Arc_CSPtr> entitytest::ArcCopy() {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();
    auto _document = new lc::DocumentImpl(_storageManager);
    auto layer = _storageManager->layerByName("0");

    auto builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset1(20., 10.);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->copy(_offset1);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset2(33., 21.);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->copy(_offset2);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset3(75., 75.);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->copy(_offset3);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _offset4(3000., 1110.);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->copy(_offset4);
    builder->execute();

    auto _entities = _document->entityContainer().asVector();
    std::vector<Arc_CSPtr> arcs;

    for (auto i : _entities) {
        arcs.push_back(std::dynamic_pointer_cast<const lc::Arc>(i));
    }

    return arcs;
}

std::vector<Arc_CSPtr> entitytest::ArcRotate() {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();
    auto _document = new lc::DocumentImpl(_storageManager);
    auto layer = _storageManager->layerByName("0");

    auto builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center1(9700., 555.);
    double angle1 = 15;
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->rotate(_center1, angle1);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center2(5825., 8078.);
    double angle2 = 105.;
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->rotate(_center2, angle2);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center3(1045., 154.);
    double angle3 = 95.;
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->rotate(_center3, angle3);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center4(0., 1550.);
    double angle4 = 72.;
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->rotate(_center4, angle4);
    builder->execute();

    auto _entities = _document->entityContainer().asVector();
    std::vector<Arc_CSPtr> arcs;

    for (auto i : _entities) {
        arcs.push_back(std::dynamic_pointer_cast<const lc::Arc>(i));
    }

    return arcs;
}

std::vector<Arc_CSPtr> entitytest::ArcScale() {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();
    auto _document = new lc::DocumentImpl(_storageManager);
    auto layer = _storageManager->layerByName("0");

    auto builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center1(120., 150.);
    lc::geo::Coordinate _ratio1(1., 10.);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->scale(_center1, _ratio1);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center2(55., 00.);
    lc::geo::Coordinate _ratio2(2., 10.);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->scale(_center2, _ratio2);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center3(0., 0.);
    lc::geo::Coordinate _ratio3(3., 3.);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->scale(_center3, _ratio3);
    builder->execute();

    builder = make_shared<lc::operation::Builder>(_document);
    lc::geo::Coordinate _center4(1., 10.);
    lc::geo::Coordinate _ratio4(1., 1.);
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0., 0.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->scale(_center4, _ratio4);
    builder->execute();

    auto _entities = _document->entityContainer().asVector();
    std::vector<Arc_CSPtr> arcs;

    for (auto i : _entities) {
        arcs.push_back(std::dynamic_pointer_cast<const lc::Arc>(i));
    }

    return arcs;
}


std::vector<Ellipse_CSPtr> entitytest::EllipseMove() {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();
    auto _document = new lc::DocumentImpl(_storageManager);
    auto layer = _storageManager->layerByName("0");

    lc::geo::Coordinate _offset1(10., 10.);
    auto builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->move(_offset1);
    builder->execute();

    lc::geo::Coordinate _offset2(10000., 15550.);
    builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->move(_offset2);
    builder->execute();


    lc::geo::Coordinate _offset3(3210., -1220.);
    builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->move(_offset3);
    builder->execute();

    lc::geo::Coordinate _offset4(-10000., -10000.);
    builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->move(_offset4);
    builder->execute();

    auto _entities = _document->entityContainer().asVector();
    std::vector<Ellipse_CSPtr> ellipses;

    for (auto i : _entities) {
        ellipses.push_back(std::dynamic_pointer_cast<const lc::Ellipse>(i));
    }

    return ellipses;
}

std::vector<Ellipse_CSPtr> entitytest::EllipseCopy() {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();
    auto _document = new lc::DocumentImpl(_storageManager);
    auto layer = _storageManager->layerByName("0");

    lc::geo::Coordinate _offset1(10., 10.);
    auto builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->copy(_offset1);
    builder->execute();

    lc::geo::Coordinate _offset2(10000., 15550.);
    builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->copy(_offset2);
    builder->execute();


    lc::geo::Coordinate _offset3(3210., -1220.);
    builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->copy(_offset3);
    builder->execute();

    lc::geo::Coordinate _offset4(-10000., -10000.);
    builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->copy(_offset4);
    builder->execute();

    auto _entities = _document->entityContainer().asVector();
    std::vector<Ellipse_CSPtr> ellipses;

    for (auto i : _entities) {
        ellipses.push_back(std::dynamic_pointer_cast<const lc::Ellipse>(i));
    }

    return ellipses;
}

std::vector<Ellipse_CSPtr> entitytest::EllipseRotate() {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();
    auto _document = new lc::DocumentImpl(_storageManager);
    auto layer = _storageManager->layerByName("0");

    lc::geo::Coordinate _center1(10., 10.);
    double angle1 = 70;
    auto builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->rotate(_center1, angle1);
    builder->execute();

    lc::geo::Coordinate _center2(10., 10.);
    double angle2 = 113;
    builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->rotate(_center2, angle2);
    builder->execute();


    lc::geo::Coordinate _center3(10., 10.);
    double angle3 = 91;
    builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->rotate(_center3, angle3);
    builder->execute();

    lc::geo::Coordinate _center4(10., 10.);
    double angle4 = 360.;
    builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->rotate(_center4, angle4);
    builder->execute();

    auto _entities = _document->entityContainer().asVector();
    std::vector<Ellipse_CSPtr> ellipses;

    for (auto i : _entities) {
        ellipses.push_back(std::dynamic_pointer_cast<const lc::Ellipse>(i));
    }

    return ellipses;
}

std::vector<Ellipse_CSPtr> entitytest::EllipseScale() {

    auto _storageManager = std::make_shared<lc::StorageManagerImpl>();
    auto _document = new lc::DocumentImpl(_storageManager);
    auto layer = _storageManager->layerByName("0");

    lc::geo::Coordinate _center1(100., 100.);
    lc::geo::Coordinate _ratio1(11., 1.);
    auto builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->scale(_center1, _ratio1);
    builder->execute();

    lc::geo::Coordinate _center2(0., 0.);
    lc::geo::Coordinate _ratio2(6., 3.);
    builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->scale(_center2, _ratio2);
    builder->execute();


    lc::geo::Coordinate _center3(-10., -10.);
    lc::geo::Coordinate _ratio3(1., 2.);
    builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->scale(_center3, _ratio3);
    builder->execute();

    lc::geo::Coordinate _center4(2700., 5000.);
    lc::geo::Coordinate _ratio4(10., 10.);
    builder = make_shared<lc::operation::Builder>(_document);
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0., 0.), lc::geo::Coordinate(100., 100.), 100, 30 * 0.0174532925, 120 * 0.0174532925, layer)).push();
    builder->scale(_center4, _ratio4);
    builder->execute();

    auto _entities = _document->entityContainer().asVector();
    std::vector<Ellipse_CSPtr> ellipses;

    for (auto i : _entities) {
        ellipses.push_back(std::dynamic_pointer_cast<const lc::Ellipse>(i));
    }

    return ellipses;
}
