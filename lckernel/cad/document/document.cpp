#include "document.h"
#include "cad/operations/documentoperation.h"

#include "cad/events/beginprocessevent.h"
#include "cad/events/commitprocessevent.h"
#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "cad/events/replaceentityevent.h"

using namespace lc;

void Document::operationStart(shared_ptr<operation::DocumentOperation> operation) {
    operation->start();
}
void Document::operationFinnish(shared_ptr<operation::DocumentOperation> operation) {
    operation->finnish();
}
void Document::operationProcess(shared_ptr<operation::DocumentOperation> operation) {
    operation->process();
}
