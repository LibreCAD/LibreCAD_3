#include "documentcanvas.h"
#include <cad/meta/metacolor.h>
#include <cad/document/document.h>
#include <cad/dochelpers/quadtree.h>
#include <cad/geometry/geoarea.h>
#include <cad/primitive/line.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/ellipse.h>
#include <cad/primitive/text.h>
#include <cad/primitive/image.h>
#include "lcdrawoptions.h"
#include "drawitems/lcvcircle.h"
#include "drawitems/lcvarc.h"
#include "drawitems/lcvdrawitem.h"
#include "drawitems/lcvline.h"
#include "drawitems/lcvellipse.h"
#include "drawitems/lcvtext.h"
#include "drawitems/lcvpoint.h"
#include "drawitems/lcdimradial.h"
#include "drawitems/lcdimdiametric.h"
#include "drawitems/lcdimlinear.h"
#include "drawitems/lcdimaligned.h"
#include "drawitems/lcdimangular.h"
#include "drawitems/lclwpolyline.h"
#include "drawitems/lcvspline.h"
#include "drawitems/lcimage.h"

#include "lcpainter.h"

#include <cad/meta/metacolor.h>
#include <cad/meta/metalinewidth.h>

#include <cad/const.h>
#include <math.h>

#include <typeinfo>

using namespace LCViewer;

DocumentCanvas::DocumentCanvas(std::shared_ptr<lc::Document> document) : _document(document), _zoomMin(0.005), _zoomMax(200.0), _deviceWidth(-1), _deviceHeight(-1), _selectedArea(nullptr), _selectedAreaIntersects(false) {


    document->addEntityEvent().connect<DocumentCanvas, &DocumentCanvas::on_addEntityEvent>(this);
    document->removeEntityEvent().connect<DocumentCanvas, &DocumentCanvas::on_removeEntityEvent>(this);
    document->commitProcessEvent().connect<DocumentCanvas, &DocumentCanvas::on_commitProcessEvent>(this);

    // Render code for selected area
    _selectedAreaPainter = [](LcPainter & painter, lc::geo::Area area , bool occupies) {
        double dashes[] = {10.0, 3.0, 3.0, 3.0};
        painter.save();
        painter.disable_antialias();
        painter.line_width(1.0);

        if (occupies) {
            painter.source_rgba(.2, .2, 1.0, .6);
        } else {
            painter.source_rgba(.2, 1.0, .2, .5);
        }

        painter.rectangle(area.minP().x(), area.minP().y(), area.width(), area.height());
        painter.fill();
        painter.rectangle(area.minP().x(), area.minP().y(), area.width(), area.height());

        if (occupies) {
            painter.source_rgba(.2, .2, 1., 0.9);
        } else {
            painter.source_rgba(.2, 1.0, .2, 0.8);
        }

        painter.set_dash(dashes, 4, 0, true);
        painter.stroke();
        painter.restore();

    };

}

DocumentCanvas::~DocumentCanvas() {

    _document->addEntityEvent().disconnect<DocumentCanvas, &DocumentCanvas::on_addEntityEvent>(this);
    _document->removeEntityEvent().disconnect<DocumentCanvas, &DocumentCanvas::on_removeEntityEvent>(this);
    _document->commitProcessEvent().disconnect<DocumentCanvas, &DocumentCanvas::on_commitProcessEvent>(this);

    for (auto i = _cachedPainters.begin(); i != _cachedPainters.end(); i++) {
        this->_deletePainterFunctor(i->second);
    }
    _cachedPainters.clear();

    if (_selectedArea != nullptr) {
        delete _selectedArea;
    }
}

void DocumentCanvas::removePainters()  {
    for (auto i = _cachedPainters.begin(); i != _cachedPainters.end(); i++) {
        this->_deletePainterFunctor(i->second);
    }
    _cachedPainters.clear();

    if (_selectedArea != nullptr) {
        delete _selectedArea;
    }
}

void DocumentCanvas::newDeviceSize(unsigned int width, unsigned int height) {
    if (_deviceWidth!=width && _deviceHeight!=height) {
        _deviceWidth = width;
        _deviceHeight = height;

        double s = 1.;
        double x = 0.;
        double y = 0.;

        if (_cachedPainters.size() != 0) {
            LcPainter* p = _cachedPainters.begin()->second;
            s = p->scale();
            p->getTranslate(&x, &y);
        }

        for (auto i = _cachedPainters.begin(); i != _cachedPainters.end(); i++) {
            _deletePainterFunctor(i->second);
            _cachedPainters[i->first] = _createPainterFunctor(_deviceWidth, _deviceHeight);
            _cachedPainters[i->first]->scale(s);
            _cachedPainters[i->first]->translate(x, y);
        }
    }
}

LcPainter& DocumentCanvas::cachedPainter(PainterCacheType cacheType) {

    double s = 1.;
    double x = 0.;
    double y = 0.;

    if (_cachedPainters.size() != 0) {
        LcPainter* p = _cachedPainters.begin()->second;
        s = p->scale();
        p->getTranslate(&x, &y);
    }

    if (_cachedPainters.count(cacheType) == 0) {
        _cachedPainters[cacheType] = _createPainterFunctor(_deviceWidth, _deviceHeight);
        _cachedPainters[cacheType]->scale(s);
        _cachedPainters[cacheType]->translate(x, y);
    }

    return *_cachedPainters[cacheType];
}

// TODO

/*
 * Code needs to be Tested.
 *
 */

void DocumentCanvas::pan(double move_x, double move_y) {

    double pan_x=0.;
    double pan_y=0.;

    if (_cachedPainters.size() != 0) {
        LcPainter* p = _cachedPainters.begin()->second;
        p->getTranslate(&pan_x, &pan_y);
    }

    /* FIXME 100.0 should be dynamically calculated, depends on the drawing speed */
    if (std::abs(pan_x-move_x) > 100.0 || std::abs(pan_y - move_y) > 100.0 || pan_x == 0.0 || pan_y == 0.0) {
        pan_x = move_x;
        pan_y = move_y;
    }

    for (auto i = _cachedPainters.begin(); i != _cachedPainters.end(); i++) {
        LcPainter* p = i->second;
        p->translate(move_x - pan_x, move_y - pan_y);
    }

    pan_x = move_x;
    pan_y = move_y;
}

void DocumentCanvas::zoom(double factor, bool relativezoom, unsigned int deviceX, unsigned int deviceY) {
    LcPainter& painter = cachedPainter(VIEWER_DOCUMENT);

    // Test for minimum and maximum zoom levels
    if ((_zoomMax <= painter.scale() && factor > 1.) || (_zoomMin >= painter.scale() && factor < 1.)) {
        return;
    }

    // Find user location at the device location
    painter.save();
    double userX = deviceX;
    double userY = deviceY;
    painter.device_to_user(&userX, &userY);
    painter.restore();

    zoom(factor, relativezoom, userX, userY, deviceX, deviceY);
}

void DocumentCanvas::zoom(double factor, bool relativezoom, double userCenterX, double userCenterY, unsigned int deviceX, unsigned int deviceY) {
    LcPainter &painter = cachedPainter(VIEWER_DOCUMENT);
    if ((_zoomMax <= painter.scale() && factor > 1.) || (_zoomMin >= painter.scale() && factor < 1.)) {
        return;
    }

    if (relativezoom) {
        factor = factor * painter.scale();
    }

    // Calculate reference device offset at device location
    painter.save();
    double refX = deviceX;
    double refY = deviceY;
    painter.reset_transformations();
    painter.scale(factor);
    painter.device_to_user(&refX, &refY);
    painter.restore();

    // Set translation
    for (auto i = _cachedPainters.begin(); i != _cachedPainters.end(); i++) {
        LcPainter *p = i->second;
        p->reset_transformations();
        p->scale(factor);
        p->translate(refX - userCenterX,-refY + userCenterY);
    }
}

void DocumentCanvas::transX(int x) {
    for (auto i = _cachedPainters.begin(); i != _cachedPainters.end(); i++) {
        LcPainter* p = i->second;
        p->translate(x, 0);
    }
}

void DocumentCanvas::transY(int y) {
    for (auto i = _cachedPainters.begin(); i != _cachedPainters.end(); i++) {
        LcPainter* p = i->second;
        p->translate(0, y);
    }
}

void DocumentCanvas::autoScale() {
    auto extends = _entityContainer.boundingBox();
    double zoom = std::min(_deviceWidth / extends.width(), _deviceHeight / extends.height());
    this->zoom(zoom, false, extends.width() / 2 + extends.minP().x(), extends.height() / 2. + extends.minP().y(), _deviceWidth / 2., _deviceHeight / 2.);
}

void DocumentCanvas::render(std::function<void(LcPainter&)> before, std::function<void(LcPainter&)> after) {

    LcPainter& painter = cachedPainter(VIEWER_DOCUMENT);
    painter = cachedPainter(VIEWER_DRAWING);
    painter = cachedPainter(VIEWER_BACKGROUND);
    lc::geo::Area visibleUserArea;

    {
        double x = 0.;
        double y = 0.;
        double w = _deviceWidth;
        double h = _deviceHeight;
        painter.device_to_user(&x, &y);
        painter.device_to_user_distance(&w, &h);
        visibleUserArea = lc::geo::Area(lc::geo::Coordinate(x, y), w, h);
    }

    // Render background
    // Cache these backgrounds
    before(painter);

    LcDrawOptions lcDrawOptions;
    DrawEvent drawEvent(painter, lcDrawOptions, visibleUserArea);
    painter.lineWidthCompensation(0.);
    _background(drawEvent);

    after(painter);

    // Draw Document
    painter = cachedPainter(VIEWER_DOCUMENT);
    before(painter);
    // caller is responsible for clearing    painter.clear(1., 1., 1., 0.);
    painter.source_rgb(1., 1., 1.);
    painter.lineWidthCompensation(0.5);
    painter.enable_antialias();

    auto visibleItems = _entityContainer.entitiesWithinAndCrossingAreaFast(visibleUserArea);

    visibleItems.each< LCVDrawItem >([&](LCVDrawItem_SPtr di) {
        std::shared_ptr<lc::entity::CADEntity> ci = std::dynamic_pointer_cast<lc::entity::CADEntity>(di);
        lc::MetaColor_CSPtr entityColor = ci->metaInfo<lc::MetaColor>(lc::MetaColor::LCMETANAME());
        lc::MetaLineWidthByValue_CSPtr entityLineWidth = ci->metaInfo<lc::MetaLineWidthByValue>(lc::MetaLineWidthByValue::LCMETANAME());
        lc::DxfLinePattern_CSPtr entityLinePattern = ci->metaInfo<lc::DxfLinePattern>(lc::DxfLinePattern::LCMETANAME());
        lc::Layer_CSPtr layer = ci->layer();

        painter.save();

        // Used to give the illusation from slightly thinner lines. Not sure yet what to d with it and if I will keep it
        double alpha_compensation = 0.9;

        // Decide on line width
        if (entityLineWidth != nullptr) {
            // We multiply for now by 3 to ensure that 1mm lines will still appear thicker on screen
            // TODO: Find a better algo
            double width = entityLineWidth->width() * 1.5;
            // Is this correct? May be we should decide on a different minimum width then 0.1, because may be on some devices 0.11 isn't visible?
            painter.line_width(std::max(width, MINIMUM_READER_LINEWIDTH));
        } else {
            // We multiply for now by 3 to ensure that 1mm lines will still appear thicker on screen
            // TODO: Find a better algo
            double width = layer->lineWidth().width() * 1.5;
            // Is this correct? May be we should decide on a different minimum width then 0.1, because may be on some devices 0.11 isn't visible?
            painter.line_width(std::max(width, MINIMUM_READER_LINEWIDTH));
        }

        if (entityLinePattern != nullptr && entityLinePattern->lcPattern().size()>0) {
            const double* path = &entityLinePattern->lcPattern()[0];
            painter.set_dash(path, entityLinePattern->lcPattern().size(), 0., true);
        }

        // Decide what color to render the entity into
        if (di->selected()) {
            painter.source_rgba(
                lcDrawOptions.selectedColor().red(),
                lcDrawOptions.selectedColor().green(),
                lcDrawOptions.selectedColor().blue(),
                lcDrawOptions.selectedColor().alpha() * alpha_compensation
            );
        } else if (entityColor != nullptr) {
            painter.source_rgba(
                entityColor->red(),
                entityColor->green(),
                entityColor->blue(),
                entityColor->alpha() * alpha_compensation);
        } else {
            lc::Color layerColor = layer->color();
            painter.source_rgba(
                layerColor.red(),
                layerColor.green(),
                layerColor.blue(),
                layerColor.alpha() * alpha_compensation);
        }



        di->draw(painter, lcDrawOptions, visibleUserArea);

        painter.restore();
    });
    painter.line_width(1.);
    painter.source_rgb(1., 1., 1.);
    painter.lineWidthCompensation(0.);
    after(painter);

    // Foreground
    painter = cachedPainter(VIEWER_DRAWING);
    before(painter);
    // caller is responsible for clearing  painter.clear(1., 1., 1., 0.0);

    _foreground(drawEvent);

    // Draw selection rectangle
    if (_selectedArea != nullptr) {
        _selectedAreaPainter(painter, *_selectedArea, _selectedAreaIntersects);
    }




    //    for (int i = 0; i < _cursorItems.size(); ++i) {
    //        this->_cursorItems.at(i)->draw(lcPainter, nullptr, rect);
    //    }

    /* Draw QuadTree (for debugging) */

    /*
    painter.save();
    painter.line_width(1.0);
    painter.disable_antialias();
    painter.source_rgba(0.7, 0.7, 1.0, .8);
    auto *t = _entityContainer.tree();
    t->walkQuad(
        [painter](const lc::QuadTreeSub<lc::entity::CADEntity_SPtr> &tree){
        lc::geo::Area a = tree.bounds();
       // painter.source_rgba(0.7, 0.7, 1.0, .8);
        painter.rectangle(a.minP().x(), a.minP().y(), a.width(), a.height());
       // painter.stroke();

        //painter.source_rgba(0.7, 1.0, .7, .8);
        //painter.rectangle(a.minP().x()+tree.level(), a.minP().y()+tree.level(), a.width()-+tree.level()*2, a.height()-+tree.level()*2);
        //painter.stroke();
    });
    painter.stroke();
    painter.restore(); */

    after(painter);

}

void DocumentCanvas::on_commitProcessEvent(const lc::CommitProcessEvent&) {
    _entityContainer.optimise();
}

void DocumentCanvas::on_addEntityEvent(const lc::AddEntityEvent& event) {

    // Add a line
    const auto line = event.entity<lc::entity::Line>();

    if (line != nullptr) {
        auto newLine = std::make_shared<LCVLine>(line);
        _entityContainer.insert(newLine);
        return;
    }

    // Add a circle
    const auto circle = event.entity<lc::entity::Circle>();

    if (circle != nullptr) {
        auto newCircle = std::make_shared<LCVCircle>(circle);
        _entityContainer.insert(std::make_shared<LCVCircle>(circle));
        return;
    }

    // Add a Arc
    const auto arc = event.entity<lc::entity::Arc>();

    if (arc != nullptr) {
        auto newArc = std::make_shared<LCVArc>(arc);
        _entityContainer.insert(newArc);
        return;
    }


    // Add Ellipse
    const auto ellipse = event.entity<lc::entity::Ellipse>();

    if (ellipse != nullptr) {
        auto newEllipse = std::make_shared<LCVEllipse>(ellipse);
        _entityContainer.insert(newEllipse);
        return;
    }

    // Add Text
    const auto text = event.entity<lc::entity::Text>();

    if (text != nullptr) {
        auto newText = std::make_shared<LCVText>(text);
        _entityContainer.insert(newText);
        return;
    }

    // Add 'Point' or 'Coordinate'
    const auto coord = event.entity<lc::entity::Point>();

    if (coord != nullptr) {
        auto newCoord = std::make_shared<LCVPoint>(coord);
        _entityContainer.insert(newCoord);
        return;
    }

    // Add 'DimRadial'
    const auto dimRadial = event.entity<lc::entity::DimRadial>();

    if (dimRadial != nullptr) {
        auto newDimRadial = std::make_shared<LCDimRadial>(dimRadial);
        _entityContainer.insert(newDimRadial);
        return;
    }

    // Add 'DimDiametric'
    const auto dimDiametric = event.entity<lc::entity::DimDiametric>();

    if (dimDiametric != nullptr) {
        auto newDimDiametric = std::make_shared<LCDimDiametric>(dimDiametric);
        _entityContainer.insert(newDimDiametric);
        return;
    }

    // Add 'DimLinear'
    const auto dimLinear = event.entity<lc::entity::DimLinear>();

    if (dimLinear != nullptr) {
        auto newDimLinear = std::make_shared<LCDimLinear>(dimLinear);
        _entityContainer.insert(newDimLinear);
        return;
    }

    // Add 'DimAligned'
    const auto dimAligned = event.entity<lc::entity::DimAligned>();

    if (dimAligned != nullptr) {
        auto newDimAligned = std::make_shared<LCDimAligned>(dimAligned);
        _entityContainer.insert(newDimAligned);
        return;
    }

    // Add 'DimAngular'
    const auto dimAngular = event.entity<lc::entity::DimAngular>();

    if (dimAngular != nullptr) {
        auto newDimAngular = std::make_shared<LCDimAngular>(dimAngular);
        _entityContainer.insert(newDimAngular);
        return;
    }

    // Add 'LWPolyline'
    const auto lwPolyline = event.entity<lc::entity::LWPolyline>();

    if (lwPolyline != nullptr) {
        auto newLLWPolyline = std::make_shared<LCLWPolyline>(lwPolyline);
        _entityContainer.insert(newLLWPolyline);
        return;
    }

    // Add 'Spline'
    const auto spline = event.entity<lc::entity::Spline>();

    if (spline != nullptr) {
        auto newSpline = std::make_shared<LCVSpline>(spline);
        _entityContainer.insert(newSpline);
        return;
    }

    // Add 'Image'
    const auto image = event.entity<lc::entity::Image>();

    if (image != nullptr) {
        auto newImage = std::make_shared<LCImage>(image);
        _entityContainer.insert(newImage);
        return;
    }
}

void DocumentCanvas::on_removeEntityEvent(const lc::RemoveEntityEvent& event) {
    auto i = _entityContainer.entityByID(event.entity()->id());
    _entityContainer.remove(i);
}

std::shared_ptr<lc::Document> DocumentCanvas::document() const {
    return _document;
}

lc::EntityContainer<lc::entity::CADEntity_SPtr> & DocumentCanvas::entityContainer() {
    return _entityContainer;
}

void DocumentCanvas::createPainterFunctor(const std::function<LcPainter *(const unsigned int, const unsigned int)>& createPainterFunctor) {
    _createPainterFunctor = createPainterFunctor;
}

void DocumentCanvas::deletePainterFunctor(const std::function<void(LcPainter*)>& deletePainterFunctor) {
    _deletePainterFunctor = deletePainterFunctor;
}

lc::geo::Area DocumentCanvas::bounds() const {
    return _entityContainer.bounds();
}

void DocumentCanvas::makeSelection(double x, double y, double w, double h, bool occupies, bool addTo) {
    if (_selectedArea != nullptr) {
        delete _selectedArea;
    }

    _selectedArea = new lc::geo::Area(lc::geo::Coordinate(x, y), lc::geo::Coordinate(x + w, y + h));
    // std::cout << *_selectedArea << std::endl;
    _selectedAreaIntersects = occupies;


    // Remove current selection
    if (!addTo) {
        _entityContainer.each< LCVDrawItem >([](LCVDrawItem_SPtr di) {
            di->selected(false);
        });
    }

    if (occupies) {
        _entityContainer.entitiesFullWithinArea(*_selectedArea).each< LCVDrawItem >([](LCVDrawItem_SPtr di) {
            // std::cerr<< __FILE__ << " : " << __FUNCTION__ << " : " << __LINE__ << " " << typeid(*di).name() << std::endl;
            di->selected(true);
        });
    } else {
        _entityContainer.entitiesWithinAndCrossingArea(*_selectedArea).each< LCVDrawItem >([](LCVDrawItem_SPtr di) {
            // std::cerr << __FILE__ << " : " << __FUNCTION__ << " : " << __LINE__ << " " << typeid(*di).name() << std::endl;
            di->selected(true);
        });

    }
}

void DocumentCanvas::makeSelectionDevice(unsigned int x, unsigned int y, unsigned int w, unsigned int h, bool occupies, bool addTo) {
    LcPainter& painter = cachedPainter(VIEWER_DOCUMENT);
    // Find mouse position in user space
    double dx = x;
    double dy = y;
    double dw = w;
    double dh = h;

    // Calculate to device space
    painter.device_to_user(&dx, &dy);
    painter.device_to_user_distance(&dw, &dh);

    // Set that as the selection of items
    makeSelection(dx, dy, dw, dh, occupies, addTo);
}


void DocumentCanvas::removeSelectionArea() {
    _entityContainer.each< LCVDrawItem >([](LCVDrawItem_SPtr di) {
        di->selected(false);
    });
    if (_selectedArea != nullptr) {
        delete _selectedArea;
        _selectedArea = nullptr;
    }
}

Nano::Signal<void(DrawEvent const & event)> & DocumentCanvas::background ()  {
    return _background;
}
Nano::Signal<void(DrawEvent const & event)> & DocumentCanvas::foreground ()  {
    return _foreground;
}

