#include "abstractdocument.h"
#include "cad/operations/documentoperation.h"

#include "cad/events/beginprocessevent.h"
#include "cad/events/commitprocessevent.h"
#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "cad/events/replaceentityevent.h"

using namespace lc;

void AbstractDocument::operationStart(shared_ptr<operation::DocumentOperation> operation) {
    operation->start();
}
void AbstractDocument::operationFinnish(shared_ptr<operation::DocumentOperation> operation) {
    operation->finnish();
}
void AbstractDocument::operationProcess(shared_ptr<operation::DocumentOperation> operation) {
    operation->process();
}
