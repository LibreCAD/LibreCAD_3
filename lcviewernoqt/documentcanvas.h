#pragma once

#include <functional>

#include "lcpainter.h"

#include "cad/dochelpers/entitycontainer.h"
#include "drawitems/lcvdrawitem.h"
#include "events/drawevent.h"
#include <cad/base/cadentity.h>

#include <cad/events/addentityevent.h>
#include <cad/events/commitprocessevent.h>
#include <cad/events/removeentityevent.h>
#include <nano-signal-slot/nano_signal_slot.hpp>
namespace LCViewer {

enum PainterCacheType {
    VIEWER_BACKGROUND,
    VIEWER_DOCUMENT,
    VIEWER_DRAWING
};


class DocumentCanvas {
    public:
        DocumentCanvas(std::shared_ptr<lc::Document> document);
        virtual ~DocumentCanvas();

        /**
         * @brief render
         * this document using it's painter's
         * The provided wPainter will allow to paint each item on the final device
         * @param wPainter
         */
        void render(std::function<void(LcPainter&)> before, std::function<void(LcPainter&)> after);

        /**
         * @brief autoScale
         * Found the bounds of the current document and scale into this and center on screen
         */
        void autoScale();

        void pan(double move_x, double move_y);

        void transX(int x);

        void transY(int y);
        /**
         * @brief zoom
         * into a specific area
         * @param factor
         * @param deviceCenterX
         * @param deviceCenterY
         */
        void zoom(double factor, bool relativezoom, unsigned int deviceCenterX, unsigned int deviceCenterY);

        /**
         * @brief zoom
         * into a specific area with a zoom factor
         * Ensures that the specified user location is located at the device location
         * on a specific zoom factor
         * @param factor
         * @param userCenterX
         * @param userCenterY
         * @param deviceCenterX
         * @param deviceCenterY
         */
        void zoom(double factor, bool relativezoom, double userCenterX, double userCenterY, unsigned int deviceCenterX, unsigned int deviceCenterY);

        /**
         * @brief newSize
         * for the device. When using a pixel based device this is the number of pixels of the painter
         * @param width
         * @param hight
         */
        void newDeviceSize(unsigned int width, unsigned int hight);

        /**
         * Remove all painter's
         * This can be usefull if your viewer that uses the document canvas get's deleted before
         * it will delete the canvas itself. This can happen if your UI layer claim's ownership
         * and you don't have the chance to control order of destruction of objects.
         */
        void removePainters();

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
         * @brief bounds
         * return the opproximate size of the current document
         * @return
         */
        lc::geo::Area bounds() const;

        /**
         * @brief makeSelection
         * within the document. It wil color the area red/green depending on the occupies flag.
         * The coordinates must be given in user coordinates
         * @param x
         * @param y
         * @param w
         * @param y
         * @param occupies
         * @param addTo -- when set to true, we add it to the current selection
         */
        void makeSelection(double x, double y, double w, double h, bool occupies, bool addTo = false);

        /**
         * @brief makeSelectionDevice
         * based on device coordinate rather then user coordinates
         * @param x
         * @param y
         * @param w
         * @param h
         * @param occupies
         * @param addTo -- when set to true, we add it to the current selection
         */
        void makeSelectionDevice(unsigned int x, unsigned int y, unsigned int w, unsigned int h, bool occupies, bool addTo = false);

        /**
        * @brief removeSelectionArea removed the colord selection area from the screen
        */
        void removeSelectionArea();


        /**
         *
         */
        void setPositionDevice(int x, int y);



        Nano::Signal<void(DrawEvent const & drawEvent)> & background () ;
        Nano::Signal<void(DrawEvent const & drawEvent)> & foreground () ;

        /**
         * I don't like this because it requires a painting context
         * However, for now I will do it because there wasn't a easy and quick way to have
         * some matrix calculations done.
         */
        void device_to_user(double *x, double *y) const {
            if (_cachedPainters.size()>0) {
                std::map<PainterCacheType, LcPainter *>::const_iterator painter = _cachedPainters.begin();
                painter->second->device_to_user(x, y);
            }
        }

        /**
         * Return the underlaying document
         */
        std::shared_ptr<lc::Document> document() const;

        /**
         * Get the current entity container,
         * don not store this as a reference, always call it
         */
        lc::EntityContainer<lc::entity::CADEntity_SPtr> & entityContainer();
private:
        /**
         * @brief cachedPainter
         * fetch a painter, if no painter was found or the painter's size doesn't match anymore
         * with the device size the old painter will request adestruction through deletePainterFunctor and
         * a new painter will be created through  createPainterFunctor
         * @param cacheType
         * @return
         */
        LcPainter& cachedPainter(PainterCacheType cacheType);

        void on_addEntityEvent(const lc::AddEntityEvent&);
        void on_removeEntityEvent(const lc::RemoveEntityEvent&);
        void on_commitProcessEvent(const lc::CommitProcessEvent&);

    private:
        // Original document
        std::shared_ptr<lc::Document> _document;

        // Local entity container
        lc::EntityContainer<lc::entity::CADEntity_SPtr> _entityContainer;

        Nano::Signal<void(DrawEvent const & event)> _background;
        Nano::Signal<void(DrawEvent const & event)> _foreground;



        // Two functor's that allow creation and destruction of painters
        std::function<LcPainter *(const unsigned int, const unsigned int)> _createPainterFunctor;
        std::function<void(LcPainter*)> _deletePainterFunctor;

        // Painters
        std::map<PainterCacheType, LcPainter*> _cachedPainters;

        // Maximum and minimum allowed scale factors
        double _zoomMin;
        double _zoomMax;

        // Current's device width and height
        unsigned int _deviceWidth;
        unsigned int _deviceHeight;

        // When !=null it show's a selected area
        lc::geo::Area* _selectedArea;

        // When set to true, a entity will be selected if it intersects or occupies,
        // when false it will only select when the entity is fully contained
        bool _selectedAreaIntersects;

        // Functor to draw a selected area, that's the green or read area...
        std::function<void(LcPainter&, lc::geo::Area, bool)> _selectedAreaPainter;

};

using DocumentCanvas_SPtr = std::shared_ptr<DocumentCanvas>;
}
