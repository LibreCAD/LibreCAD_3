#include "document.h"
#include "cad/operations/documentoperation.h"

#include "cad/events/beginprocessevent.h"
#include "cad/events/commitprocessevent.h"
#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "cad/events/replaceentityevent.h"

using namespace lc;

void Document::operationStart(std::shared_ptr<operation::DocumentOperation> operation) {
    operation->start();
}
void Document::operationFinnish(std::shared_ptr<operation::DocumentOperation> operation) {
    operation->finnish();
}
void Document::operationProcess(std::shared_ptr<operation::DocumentOperation> operation) {
    operation->process();
}
