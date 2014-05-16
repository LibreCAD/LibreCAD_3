#include "document.h"
#include "cad/operations/documentoperation.h"

#include "cad/events/beginprocessevent.h"
#include "cad/events/commitprocessevent.h"
#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "cad/events/replaceentityevent.h"

using namespace lc;

void Document::operationStart(operation::DocumentOperation_SPtr operation) {
    operation->start();
}
void Document::operationFinnish(operation::DocumentOperation_SPtr operation) {
    operation->finnish();
}
void Document::operationProcess(operation::DocumentOperation_SPtr operation) {
    operation->process();
}
