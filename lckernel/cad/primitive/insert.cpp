#include "insert.h"

using namespace lc;
using namespace entity;

Insert::Insert(Insert_CSPtr other, bool sameID) :
    CADEntity(other, sameID),
    _document(other->_document),
    _position(other->_position),
    _displayBlock(other->_displayBlock) {

}

Insert::Insert(const builder::InsertBuilder& builder) :
    CADEntity(builder),
    _document(builder.document()),
    _position(builder.coordinate()),
    _displayBlock(builder.displayBlock()) {
}


const Block_CSPtr& Insert::displayBlock() const {
    return _displayBlock;
}

const geo::Coordinate& Insert::position() const {
    return _position;
}

CADEntity_CSPtr Insert::move(const geo::Coordinate& offset) const {
    auto newEntity = std::make_shared<Insert>(shared_from_this(), true);
    newEntity->_position = _position + offset;

    return newEntity;
}

CADEntity_CSPtr Insert::copy(const geo::Coordinate& offset) const {
    auto newEntity = std::make_shared<Insert>(shared_from_this());
    newEntity->_position = _position + offset;

    return newEntity;
}

CADEntity_CSPtr Insert::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    //TODO
    return shared_from_this();
}

CADEntity_CSPtr Insert::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    //TODO
    return shared_from_this();
}

CADEntity_CSPtr Insert::mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const {
    //TODO
    return shared_from_this();
}

const geo::Area Insert::boundingBox() const {
    geo::Area area;

    for(auto entity : _document->entitiesByBlock(_displayBlock).asVector()) {
        area.merge(entity->boundingBox());
    }

    return area;
}

CADEntity_CSPtr Insert::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    //TODO
    return shared_from_this();
}

void Insert::dispatch(EntityDispatch& dispatch) const {
    //TODO
}

std::map<unsigned int, geo::Coordinate> entity::Insert::dragPoints() const {
    auto result = std::map<unsigned int, geo::Coordinate>();

    result.insert(std::pair<unsigned int, geo::Coordinate>(0, _position));

    return result;
}

entity::CADEntity_CSPtr entity::Insert::setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const {
    try {
        auto newEntity = std::make_shared<Insert>(shared_from_this(), true);
        newEntity->_position = dragPoints.at(0);

        return newEntity;
    }
    catch(std::out_of_range& e) {
        return shared_from_this();
    }
}

std::vector<lc::EntityCoordinate> entity::Insert::snapPoints(const geo::Coordinate& coord,
                                                             const SimpleSnapConstrain& simpleSnapConstrain,
                                                             double minDistanceToSnap,
                                                             int maxNumberOfSnapPoints) const {
    std::vector<EntityCoordinate> points;

    if (simpleSnapConstrain.constrain() & SimpleSnapConstrain::LOGICAL) {
        points.emplace_back(_position, 0);
    }

    Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);
    return points;
}

geo::Coordinate Insert::nearestPointOnPath(const geo::Coordinate& coord) const {
    return _position;
}

const Document_SPtr& Insert::document() const {
    return _document;
}
