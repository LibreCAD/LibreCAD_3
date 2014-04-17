#include "rotate.h"
#include "cad/document/abstractdocument.h"

using namespace lc;
using namespace lc::operation;

Rotate::Rotate(AbstractDocument* document, const geo::Coordinate& rotation_center, const double rotation_angle, const long no_copies) : Operation(document), Undoable("Create entities") {
    _rotation_center = rotation_center;
    _rotation_angle = rotation_angle;
    _no_copies = no_copies;
}

void Rotate::append(shared_ptr<const lc::CADEntity> cadEntity) {
    _toRotate.append(cadEntity);
}

void Rotate::processInternal() {    
    for (int a = 1; a <= _no_copies; ++a) {
        for (int i = 0; i < _toRotate.size(); ++i) {
            auto entity = _toRotate.at(i)->rotate(_rotation_center, _rotation_angle * a);
            _newlyCreated.append(entity);
            document()->removeEntity(_toRotate.at(i)->id());
            document()->addEntity(document()->findEntityLayerByID(_toRotate.at(i)->id()), entity);
        }
    }
}

void Rotate::undo() const {

}

void Rotate::redo() const {

}
