#ifndef SELECTIONMANAGERIMPL_H
#define SELECTIONMANAGERIMPL_H

#include "cad/const.h"

#include "cad/document/selectionmanager.h"
#include "cad/document/layermanager.h"

namespace lc {
    class SelectionManagerImpl : public SelectionManager {
        public:
            SelectionManagerImpl(LayerManager* layerManager, double _distance);

            virtual void getEntitiesNearCoordinate(const geo::Coordinate& point) const;
            virtual void getEntitiesInArea(const geo::Area& area) const;

        private:
            double _distance;
            LayerManager* _layerManager;
    };
}
#endif // SELECTIONMANAGERIMPL_H
