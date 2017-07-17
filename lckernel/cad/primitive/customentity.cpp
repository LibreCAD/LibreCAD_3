#include "customentity.h"

lc::entity::CustomEntity::CustomEntity(const lc::builder::InsertBuilder& builder) : Insert(builder) {

}

lc::entity::CustomEntity::CustomEntity(lc::entity::Insert_CSPtr insert, bool sameID) : Insert(insert, sameID) {

}
