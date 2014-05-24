#ifndef DOCUMENTRENDERER_H
#define DOCUMENTRENDERER_H

#include <functional>

#include "lcpainter.h"

#include <cad/dochelpers/entitycontainer.h>
#include <drawitems/lcvdrawitem.h>
#include <cad/base/cadentity.h>

#include <cad/events/addentityevent.h>
#include <cad/events/commitprocessevent.h>
#include <cad/events/removeentityevent.h>

enum PainterCacheType {
    VIEWER_BACKGROUND,
    VIEWER_DOCUMENT,
    VIEWER_DRAWING
};


class DocumentRenderer {
    public:
        DocumentRenderer(lc::Document* _document);
        ~DocumentRenderer();

        /**
         * @brief render
         * this document using it's painter's
         * The provided wPainter will allow to paint each item on the final device
         * @param wPainter
         */
        void render(std::function<void(LcPainter*)> wPainter);

        void scrollTo(double factor, unsigned int deviceCenterX, unsigned int deviceCenterY);
        /**
         * @brief newSize
         * for the device. When using a pixel based device this is the number of pixels of the painter
         * @param width
         * @param hight
         */
        void newDeviceSize(unsigned int width, unsigned int hight);

        /**
         * @brief createPainterFunctor
         * is called each time a new LcPainter is required. The underlaying implementation allow's you to decide
         * into what the document get's painted.
         * @param createPainterFunctor
         */
        void createPainterFunctor(const std::function<LcPainter *(const unsigned int, const unsigned int)>& createPainterFunctor);

        /**
         * @brief deletePainterFunctor
         * called when the LcPainter is no longer needed
         * @param deletePainterFunctor
         */
        void deletePainterFunctor(const std::function<void(LcPainter*)>& deletePainterFunctor);

        /**
         * @brief addBackgroundItem
         * to this document. The background will be painted first. If no background items are added the document get cleared out
         * @param item
         */
        void addBackgroundItem(std::shared_ptr<LCVDrawItem> item);
        /**
         * @brief addForegroundItem
         * to this document. Forground items must be transparent (alpha == 0) and will needs to be 'light'
         * this painter takes care of drawing grids and cursors
         * @param item
         */
        void addForegroundItem(std::shared_ptr<LCVDrawItem> item);

        /**
         * @brief bounds
         * return the opproximate size of the current document
         * @return
         */
        lc::geo::Area bounds() const;

        /**
         * @brief makeSelection
         * within the document. It wil color the area red/green depending on the occupies flag
         * @param x
         * @param y
         * @param w
         * @param y
         * @param foo
         */
        void makeSelection(double x, dobule y, double w, double y, bool occupies);

        }

         /**
         * @brief removeSelectionArea removed the colord selection area from the screen
         */
        void removeSelectionArea();

    private:
        /**
         * @brief cachedPainter
         * fetch a painter, if no painter was found or the painter's size doesn't match anymore
         * with the device size the old painter will request adestruction through deletePainterFunctor and
         * a new painter will be created through  createPainterFunctor
         * @param cacheType
         * @return
         */
        LcPainter* cachedPainter(PainterCacheType cacheType);

        void on_addEntityEvent(const lc::AddEntityEvent&);
        void on_removeEntityEvent(const lc::RemoveEntityEvent&);
        void on_commitProcessEvent(const lc::CommitProcessEvent&);

    private:
        // Original document
        lc::Document* _document;

        // Local entity container
        lc::EntityContainer _entityContainer;

        // Background and forground items
        std::vector<std::shared_ptr<LCVDrawItem> > _backgroundItems;
        std::vector<std::shared_ptr<LCVDrawItem> > _foregroundItems;

        // Two functor's that allow
        std::function<LcPainter *(const unsigned int, const unsigned int)> _createPainterFunctor;
        std::function<void(LcPainter*)> _deletePainterFunctor;

        // Painters
        std::map<PainterCacheType, LcPainter*> _cachedPainters;

        // Maximum and minimum allowed scale factors
        double _zoomMin;
        double _zoomMax;

        // Area visible for the user
        lc::geo::Area _visibleUserArea;

        // Current's device width and height
        unsigned int _deviceWidth;
        unsigned int _deviceHeight;
};


#endif // DOCUMENTRENDERER_H
