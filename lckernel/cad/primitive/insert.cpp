#include "insert.h"

using namespace lc;
using namespace entity;

Insert::Insert(const Insert_CSPtr& other, bool sameID) :
    CADEntity(other, sameID),
    _document(other->_document),
    _position(other->_position),
    _displayBlock(other->_displayBlock) {

    calculateBoundingBox();

    _document->addEntityEvent().connect<Insert, &Insert::on_addEntityEvent>(this);
    _document->removeEntityEvent().connect<Insert, &Insert::on_removeEntityEvent>(this);
}

Insert::Insert(const builder::InsertBuilder& builder) :
    CADEntity(builder),
    _document(builder.document()),
    _position(builder.coordinate()),
    _displayBlock(builder.displayBlock()) {

    calculateBoundingBox();

    _document->addEntityEvent().connect<Insert, &Insert::on_addEntityEvent>(this);
    _document->removeEntityEvent().connect<Insert, &Insert::on_removeEntityEvent>(this);
}

Insert::~Insert() {
    document()->addEntityEvent().disconnect<Insert, &Insert::on_addEntityEvent>(this);
    document()->removeEntityEvent().disconnect<Insert, &Insert::on_removeEntityEvent>(this);
}

const meta::Block_CSPtr& Insert::displayBlock() const {
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

CADEntity_CSPtr Insert::rotate(const geo::Coordinate& rotation_center, double rotation_angle) const {
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
    return _boundingBox;
}

CADEntity_CSPtr Insert::modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto builder = builder::InsertBuilder();

    builder.copy(shared_from_this());
    builder.setLayer(std::move(layer));
    builder.setMetaInfo(std::move(metaInfo));
    builder.setBlock(std::move(block));

    return builder.build();
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

    if ((bool) (simpleSnapConstrain.constrain() & SimpleSnapConstrain::LOGICAL)) {
        points.emplace_back(_position, 0);
    }

    Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);
    return points;
}

geo::Coordinate Insert::nearestPointOnPath(const geo::Coordinate& coord) const {
    return _position;
}

const storage::Document_SPtr& Insert::document() const {
    return _document;
}

void Insert::calculateBoundingBox() {
    auto entities = _document->entitiesByBlock(_displayBlock).asVector();

    if(entities.empty()) {
        _boundingBox = geo::Area(_position, _position);
        return;
    }

    auto offset = _position - displayBlock()->base();
    auto it = entities.begin();

    _boundingBox = (*it)->move(offset)->boundingBox();
    it++;

    while (it != entities.end()) {
        _boundingBox = _boundingBox.merge((*it)->move(offset)->boundingBox());
        it++;
    }
}

void Insert::on_addEntityEvent(const lc::event::AddEntityEvent& event) {
    if(event.entity()->block() == _displayBlock) {
        calculateBoundingBox();
    }
}

void Insert::on_removeEntityEvent(const lc::event::RemoveEntityEvent& event) {
    if(event.entity()->block() == _displayBlock) {
        calculateBoundingBox();
    }
}