#include "id.h"

using namespace lc;

ID::ID() {
    static unsigned long idCounter = 1;
    _id = idCounter++;
}

ID_DATATYPE ID::id() const {
    return _id;
}

void ID::setID(ID_DATATYPE _ID) {
    _id = _ID;
}
