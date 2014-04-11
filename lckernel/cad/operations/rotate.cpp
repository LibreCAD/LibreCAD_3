#include "rotate.h"
#include "cad/document/abstractdocument.h"

using namespace lc;
using namespace lc::operation;

Rotate::Rotate(AbstractDocument* document, const geo::Coordinate& offset, const geo::Coordinate& rotation_center, const double& rotation_angle, const long& no_of_operations) : Operation(document), Undoable("Create entities") {
    _offset = offset;
    _rotation_center = rotation_center;
    _rotation_angle = rotation_angle;
    _no_of_operations = no_of_operations;
}

void Rotate::append(shared_ptr<const lc::CADEntity> cadEntity) {
    _toRotate.append(cadEntity);
}

void Rotate::processInternal() const {
    for (int i = 0; i < _toRotate.size(); ++i) {

        // Orignal entity will only be removed if no of operations is greater than 0
        if(_no_of_operations > 0 ) {
            document()->removeEntity(_toRotate.at(i)->id());
        }
        double current_angle = _rotation_angle;
        geo::Coordinate current_rotation_center = _rotation_center;
        geo::Coordinate current_offset = _offset;
        for (int a = 0; a < _no_of_operations; ++a) {
            /* for the first operation the ID of orignal entity is copied. */
            if (a == 1) {
                document()->addEntity(document()->findEntityLayerByID(_toRotate.at(i)->id()), _toRotate.at(i)->rotate(current_offset, current_rotation_center, current_angle, 1));
            } else {
                /* A new ID is given to next operations */
                document()->addEntity(document()->findEntityLayerByID(_toRotate.at(i)->id()), _toRotate.at(i)->rotate(current_offset, current_rotation_center, current_angle, 0));
            }
            current_angle = current_angle + _rotation_angle;
            current_rotation_center = current_rotation_center + _rotation_center;
            current_offset = current_offset + _offset;
        }
    }
}

void Rotate::undo() const {
    for (int i = 0; i < _toRotate.size(); ++i) {
        if(_no_of_operations > 0 ) {
            document()->addEntity(document()->findEntityLayerByID(_toRotate.at(i)->id()), _toRotate.at(i));
        }
        if (_no_of_operations == 1) {
            document()->removeEntity(_toRotate.at(i)->id());
        } else {
            for (int a = 0; a < _no_of_operations; ++a) {
                document()->removeEntity(_toRotate.at(a)->id());
            }
        }
    }
}

void Rotate::redo() const {
    processInternal();
}
