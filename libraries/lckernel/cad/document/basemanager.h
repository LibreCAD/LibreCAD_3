#ifndef BASEMANAGER_H
#define BASEMANAGER_H

#include <QObject>

namespace lc {

    class AbstractDocument;
    class BaseManager : public QObject {
            Q_OBJECT
        public:
            virtual AbstractDocument* Document() const {
                return _document;
            };
            virtual void document(AbstractDocument* document) {
                _document = document;
            };
        private:
            AbstractDocument* _document;

    };
}
#endif // BASEMANAGER_H
