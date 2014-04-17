#include "copy.h"
#include "cad/document/abstractdocument.h"

using namespace lc;
using namespace lc::operation;

Copy::Copy(AbstractDocument* document, const geo::Coordinate& offset, const geo::Coordinate& rotation_center, const double rotation_angle, const long no_of_operations) : Operation(document), Undoable("Create entities") {
    _offset = offset;
    _rotation_center = rotation_center;
    _rotation_angle = rotation_angle;
    _no_of_operations = no_of_operations;
}

void Copy::append(shared_ptr<const CADEntity> cadEntity) {
    _toCopy.append(cadEntity);
}

void Copy::processInternal() {
    for (int i = 0; i < _toCopy.size(); ++i) {
        geo::Coordinate current_offset = _offset;
        for (int a = 0; a < _no_of_operations; ++a) {
            document()->addEntity(document()->findEntityLayerByID(_toCopy.at(i)->id()), _toCopy.at(i)->copy(current_offset));
            current_offset = current_offset + _offset;
        }
    }
}

void Copy::undo() const {
    for (int i = 0; i < _toCopy.size(); ++i) {
        for (int a = 0; a < _no_of_operations; ++a) {
            document()->removeEntity(_toCopy.at(a)->id());
        }
    }
}

void Copy::redo() const {
    for (int i = 0; i < _toCopy.size(); ++i) {
        geo::Coordinate current_offset = _offset;
        for (int a = 0; a < _no_of_operations; ++a) {
            document()->addEntity(document()->findEntityLayerByID(_toCopy.at(i)->id()), _toCopy.at(i)->copy(current_offset));
            current_offset = current_offset + _offset;
        }
    }
}
