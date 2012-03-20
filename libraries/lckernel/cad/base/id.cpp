#include "id.h"

using namespace lc;

ID::ID() {
    static unsigned long idCounter = 1;
    _id = idCounter++;
}

UNIQUE_CAD_ID ID::id() const {
    return _id;
}
