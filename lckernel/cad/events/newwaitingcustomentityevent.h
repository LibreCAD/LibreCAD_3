#pragma once

#include "cad/operations/documentoperation.h"

namespace lc {
    class NewWaitingCustomEntityEvent {
        public:
            NewWaitingCustomEntityEvent(const entity::Insert_CSPtr& insert) :
                    _insert(insert) {

            }

            const entity::Insert_CSPtr& insert() const {
                return _insert;
            }

        private:
            entity::Insert_CSPtr _insert;
    };
}