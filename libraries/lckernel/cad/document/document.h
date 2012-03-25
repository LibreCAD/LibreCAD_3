#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QString>
#include "layermanager.h"
#include "selectionmanager.h"
#include "cad/geometry/coordinate.h"
#include "cad/base/cadentity.h"

namespace lc {
    class Document : public QObject {
            Q_OBJECT
        public:

            virtual LayerManager* layerManager() const = 0;
    };

}

#endif // DOCUMENT_H
