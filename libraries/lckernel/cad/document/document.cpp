#include "document.h"
#include "cad/operations/operation.h"

using namespace lc;

void Document::operateOn(Operation* operation) {
    begin(operation);
    operation->process(this);
    commit(operation);
}

void Document::begin(Operation* operation) {
    lock();
    operation->begin();
}
void Document::commit(Operation* operation) {
    operation->commit();
    releaseLock();
}
