#include "id.h"

using namespace lc;

std::atomic<ID_DATATYPE> ID::__idCounter(1);

ID::ID() {
    _id = ++ID::__idCounter;
}
ID::ID(ID_DATATYPE id) {
    if (id == 0) {
        _id = ++ID::__idCounter;
    } else {
        _id = id;
    }
}

ID_DATATYPE ID::id() const {
    return _id;
}

void ID::setID(ID_DATATYPE id) {
    _id = id;
}
