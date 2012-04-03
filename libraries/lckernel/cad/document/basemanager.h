#ifndef BASEMANAGER_H
#define BASEMANAGER_H

#include <QObject>

#include "cad/const.h"


namespace lc {

    class AbstractDocument;
    class BaseManager : public QObject {
            Q_OBJECT
        public:
            BaseManager() {
                _document = NULL;
            }
            BaseManager(AbstractDocument* document) {
                _document = document;
            }

            virtual void setDocument(AbstractDocument* document) {
                _document = document;
            };

            virtual AbstractDocument* document() {
                return _document;
            };
        private:
            AbstractDocument* _document;

    };
}
#endif // BASEMANAGER_H
