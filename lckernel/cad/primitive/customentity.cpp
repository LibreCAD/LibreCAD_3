#include "customentity.h"

lc::entity::CustomEntity::CustomEntity(const lc::builder::InsertBuilder& builder) : Insert(builder) {
}

lc::entity::CustomEntity::CustomEntity(const lc::entity::Insert_CSPtr& insert, bool sameID) :
    Insert(insert, sameID) {
}

lc::entity::CADEntity_CSPtr lc::entity::CustomEntity::setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const {
    return shared_from_this();
}
