#include "abstractdocument.h"
#include "cad/operations/operation.h"

#include "cad/events/beginprocessevent.h"
#include "cad/events/commitprocessevent.h"
#include "cad/events/addentityevent.h"
#include "cad/events/removeentityevent.h"
#include "cad/events/replaceentityevent.h"

using namespace lc;

void AbstractDocument::operationStart(boost::shared_ptr<lc::Operation> operation) {
    operation->start();
}
void AbstractDocument::operationFinnish(boost::shared_ptr<lc::Operation> operation) {
    operation->finnish();
}
void AbstractDocument::operationProcess(boost::shared_ptr<lc::Operation> operation) {
    operation->process();
}
