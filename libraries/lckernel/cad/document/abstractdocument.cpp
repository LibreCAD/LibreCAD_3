#include "abstractdocument.h"
#include "cad/operations/operation.h"

#include "cad/events/beginprocessevent.h"
#include "cad/events/commitprocessevent.h"
#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "cad/events/replaceentityevent.h"

using namespace lc;

void AbstractDocument::operationStart(OperationPtr operation) {
    operation->start();
}
void AbstractDocument::operationFinnish(OperationPtr operation) {
    operation->finnish();
}
void AbstractDocument::operationProcess(OperationPtr operation) {
    operation->process();
}
