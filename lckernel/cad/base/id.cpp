#include "id.h"

using namespace lc;

std::atomic<ID_DATATYPE> ID::__idCounter;

ID::ID(){
    _id = ID::__idCounter++;
}

ID_DATATYPE ID::id() const {
    return _id;
}

void ID::setID(ID_DATATYPE _ID) {
    _id = _ID;
}
