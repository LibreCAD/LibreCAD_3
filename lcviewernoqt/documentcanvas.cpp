#include "documentcanvas.h"
#include <cad/meta/metacolor.h>
#include <cad/document/document.h>
#include <cad/dochelpers/quadtree.h>
#include <cad/geometry/geoarea.h>
#include <cad/primitive/line.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/ellipse.h>
#include <cad/primitive/text.h>
#include "drawitems/lcdrawoptions.h"
#include "drawitems/lcvcircle.h"
#include "drawitems/lcvarc.h"
#include "drawitems/lcvdrawitem.h"
#include "drawitems/lcvline.h"
#include "drawitems/lcvellipse.h"
#include "drawitems/lcvtext.h"
#include "drawitems/lcvcoordinate.h"
#include "drawitems/lcdimradial.h"
#include "drawitems/lcdimdiametric.h"

#include "lcpainter.h"

#include <cad/meta/metacolor.h>
#include <cad/meta/metalinewidth.h>

#include <cad/const.h>

#include <typeinfo>

DocumentCanvas::DocumentCanvas(lc::Document* document) : _document(document), _zoomMin(0.005), _zoomMax(200.0), _deviceWidth(-1), _deviceHeight(-1), _selectedArea(nullptr), _selectedAreaIntersects(false) {


    document->addEntityEvent().connect<DocumentCanvas, &DocumentCanvas::on_addEntityEvent>(this);
    document->removeEntityEvent().connect<DocumentCanvas, &DocumentCanvas::on_removeEntityEvent>(this);
    document->commitProcessEvent().connect<DocumentCanvas, &DocumentCanvas::on_commitProcessEvent>(this);

    // Render code for elected area
    _selectedAreaPainter = [](LcPainter * painter, lc::geo::Area area , bool occupies) {
        double dashes[] = {10.0, 3.0, 3.0, 3.0};
        painter->save();
        painter->disable_antialias();
        painter->line_width(1.0);

        if (occupies) {
            painter->source_rgba(.2, .2, 1.0, .6);
        } else {
            painter->source_rgba(.2, 1.0, .2, .5);
        }

        painter->rectangle(area.minP().x(), area.minP().y(), area.width(), area.height());
        painter->fill();
        painter->rectangle(area.minP().x(), area.minP().y(), area.width(), area.height());

        if (occupies) {
            painter->source_rgba(.2, .2, 1., 0.9);
        } else {
            painter->source_rgba(.2, 1.0, .2, 0.8);
        }

        painter->set_dash(dashes, 4, 0, true);
        painter->stroke();
        painter->restore();

    };

}

DocumentCanvas::~DocumentCanvas() {

    _document->addEntityEvent().disconnect<DocumentCanvas, &DocumentCanvas::on_addEntityEvent>(this);
    _document->removeEntityEvent().disconnect<DocumentCanvas, &DocumentCanvas::on_removeEntityEvent>(this);
    _document->commitProcessEvent().disconnect<DocumentCanvas, &DocumentCanvas::on_commitProcessEvent>(this);

    for (auto i = _cachedPainters.begin(); i != _cachedPainters.end(); i++) {
        this->_deletePainterFunctor(i->second);
    }

    if (_selectedArea != nullptr) {
        delete _selectedArea;
    }
}

void DocumentCanvas::newDeviceSize(unsigned int width, unsigned int height) {

    if (_deviceWidth != width || _deviceHeight != height) {
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

    calculateVisibleUserArea();
}

LcPainter* DocumentCanvas::cachedPainter(PainterCacheType cacheType) {

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

    return _cachedPainters[cacheType];
}


void DocumentCanvas::zoom(double factor, unsigned int deviceScrollX, unsigned int deviceScrollY) {
    LcPainter* painter = cachedPainter(VIEWER_DOCUMENT);

    // Test for minimum and maximum zoom levels
    if ((_zoomMax <= painter->scale() && factor > 1.) || (_zoomMin >= painter->scale() && factor < 1.)) {
        return;
    }

    painter->save();
    // Find mouse position in user space
    double userScrollX = deviceScrollX;
    double userScrollY = deviceScrollY;
    painter->device_to_user(&userScrollX, &userScrollY);

    painter->scale(factor);

    // Find out how much the offset was after scale
    double userCenterX = deviceScrollX;
    double userCenterY = deviceScrollY;
    painter->device_to_user(&userCenterX, &userCenterY);
    painter->restore();

    // Set translation
    for (auto i = _cachedPainters.begin(); i != _cachedPainters.end(); i++) {
        LcPainter* p = i->second;
        p->translate(-userScrollX + userCenterX, userScrollY - userCenterY);
        p->scale(factor);
    }

    // Calculate visible area
    calculateVisibleUserArea();
}

/**
* I admit it, it doesn't auto scale yet and it's on my TODO to fix that
*/
void DocumentCanvas::autoScale() {

    // Set translation
    for (auto i = _cachedPainters.begin(); i != _cachedPainters.end(); i++) {
        LcPainter* p = i->second;
        p->reset_transformations();
        p->scale(1.);
        p->translate(_deviceWidth / 2., _deviceHeight / 2.);
    }

    calculateVisibleUserArea();
}

void DocumentCanvas::calculateVisibleUserArea() {
    if (_cachedPainters.size() != 0) {
        LcPainter* p = _cachedPainters.begin()->second;
        double x = 0.;
        double y = 0.;
        double w = _deviceWidth;
        double h = _deviceHeight;
        p->device_to_user(&x, &y);
        p->device_to_user_distance(&w, &h);


        _visibleUserArea = lc::geo::Area(lc::geo::Coordinate(x, y), w, h);
    } else {
        _visibleUserArea = lc::geo::Area(lc::geo::Coordinate(0, 0), 0, 0);
    }
}

void DocumentCanvas::render(std::function<void(LcPainter*)> before, std::function<void(LcPainter*)> after) {

    LcPainter* painter = cachedPainter(VIEWER_DOCUMENT);

    if (_visibleUserArea.width() == 0) {
        painter = cachedPainter(VIEWER_DRAWING);
        painter = cachedPainter(VIEWER_BACKGROUND);
        autoScale();
    }

    // Render background
    // Cache these backgrounds
    painter = cachedPainter(VIEWER_BACKGROUND);
    before(painter);

    if (_backgroundItems.size() == 0) {
        // caller is responsible for clearing       painter->clear(0., 0.1, 0.);
    }

    for (auto item : _backgroundItems) {
        item->draw(painter, nullptr, _visibleUserArea);
    }

    after(painter);

    // Draw Document
    painter = cachedPainter(VIEWER_DOCUMENT);
    before(painter);
    // caller is responsible for clearing    painter->clear(1., 1., 1., 0.);
    painter->source_rgb(1., 1., 1.);
    painter->lineWidthCompensation(0.5);

    LcDrawOptions lcDrawOptions;

    calculateVisibleUserArea();
    auto visibleItems = _entityContainer.entitiesWithinAndCrossingAreaFast(_visibleUserArea);

    visibleItems.each< LCVDrawItem >([&](LCVDrawItem_SPtr di) {
        bool modified = false;

        std::shared_ptr<lc::CADEntity> ci = std::dynamic_pointer_cast<lc::CADEntity>(di);
        lc::MetaColor_CSPtr entityColor = ci->metaInfo<lc::MetaColor>(lc::MetaColor::LCMETANAME());
        lc::MetaLineWidth_CSPtr entityLineWidth = ci->metaInfo<lc::MetaLineWidth>(lc::MetaLineWidth::LCMETANAME());
        lc::Layer_CSPtr layer = ci->layer();

        modified = true;
        painter->save();

        // Decide on line width
        if (entityLineWidth != nullptr) {
            double width = entityLineWidth->width() * 3.;
            // Is this correct? May be we should decide on a different minimum width then 0.1, because may be on some devices 0.11 isn't visible?
            painter->line_width(width < 0.1 ? 1. : width);
        } else {
            double width = layer->lineWidth().width() * 3.;
            // Is this correct? May be we should decide on a different minimum width then 0.1, because may be on some devices 0.11 isn't visible?
            painter->line_width(width < 0.1 ? 1. : width);
        }

        // Decide what color to render the entity into
        if (di->selected()) {
            painter->source_rgba(
                lcDrawOptions.selectedColor().red(),
                lcDrawOptions.selectedColor().green(),
                lcDrawOptions.selectedColor().blue(),
                lcDrawOptions.selectedColor().alpha()
            );
        } else if (entityColor != nullptr) {
            painter->source_rgba(
                entityColor->red(),
                entityColor->green(),
                entityColor->blue(),
                entityColor->alpha());
        } else {
            lc::Color layerColor = layer->color();
            painter->source_rgba(
                layerColor.red(),
                layerColor.green(),
                layerColor.blue(),
                layerColor.alpha());

        }

        di->draw(painter, &lcDrawOptions, _visibleUserArea);

        if (modified) {
            painter->restore();
        }
    });
    after(painter);

    // Foreground
    painter = cachedPainter(VIEWER_DRAWING);
    before(painter);
    // caller is responsible for clearing  painter->clear(1., 1., 1., 0.0);

    for (auto item : _foregroundItems) {
        item->draw(painter, nullptr, _visibleUserArea);
    }

    // Draw selection rectangle
    if (_selectedArea != nullptr) {
        _selectedAreaPainter(painter, *_selectedArea, _selectedAreaIntersects);
    }

    //    for (int i = 0; i < _cursorItems.size(); ++i) {
    //        this->_cursorItems.at(i)->draw(lcPainter, nullptr, rect);
    //    }

    /* Draw QuadTree (for debugging) */

    /*
    painter->save();
    painter->line_width(1.0);
    painter->disable_antialias();
    painter->source_rgba(0.7, 0.7, 1.0, .8);
    auto *t = _entityContainer.tree();
    t->walkQuad(
        [painter](const lc::QuadTreeSub<lc::CADEntity_SPtr> &tree){
        lc::geo::Area a = tree.bounds();
       // painter->source_rgba(0.7, 0.7, 1.0, .8);
        painter->rectangle(a.minP().x(), a.minP().y(), a.width(), a.height());
       // painter->stroke();

        //painter->source_rgba(0.7, 1.0, .7, .8);
        //painter->rectangle(a.minP().x()+tree.level(), a.minP().y()+tree.level(), a.width()-+tree.level()*2, a.height()-+tree.level()*2);
        //painter->stroke();
    });
    painter->stroke();
    painter->restore(); */

    after(painter);

}

void DocumentCanvas::addBackgroundItem(std::shared_ptr<LCVDrawItem> item) {
    this->_backgroundItems.push_back(item);
}

void DocumentCanvas::addForegroundItem(std::shared_ptr<LCVDrawItem> item) {
    this->_foregroundItems.push_back(item);
}

void DocumentCanvas::on_commitProcessEvent(const lc::CommitProcessEvent&) {
    _entityContainer.optimise();
}

void DocumentCanvas::on_addEntityEvent(const lc::AddEntityEvent& event) {

    // Add a line
    const auto line = event.entity<lc::Line>();

    if (line != nullptr) {
        auto newLine = std::make_shared<LCVLine>(line);
        _entityContainer.insert(newLine);
        return;
    }

    // Add a circle
    const auto circle = event.entity<lc::Circle>();

    if (circle != nullptr) {
        auto newCircle = std::make_shared<LCVCircle>(circle);
        _entityContainer.insert(std::make_shared<LCVCircle>(circle));
        return;
    }

    // Add a Arc
    const auto arc = event.entity<lc::Arc>();

    if (arc != nullptr) {
        auto newArc = std::make_shared<LCVArc>(arc);
        _entityContainer.insert(newArc);
        return;
    }


    // Add Ellipse
    const auto ellipse = event.entity<lc::Ellipse>();

    if (ellipse != nullptr) {
        auto newEllipse = std::make_shared<LCVEllipse>(ellipse);
        _entityContainer.insert(newEllipse);
        return;
    }

    // Add Text
    const auto text = event.entity<lc::Text>();

    if (text != nullptr) {
        auto newText = std::make_shared<LCVText>(text);
        _entityContainer.insert(newText);
        return;
    }

    // Add 'Point' or 'Coordinate'
    const auto coord = event.entity<lc::Coordinate>();

    if (coord != nullptr) {
        auto newCoord = std::make_shared<LCVCoordinate>(coord);
        _entityContainer.insert(newCoord);
        return;
    }

    // Add 'DimRadial'
    const auto dimRadial = event.entity<lc::DimRadial>();

    if (dimRadial != nullptr) {
        auto newDimRadial = std::make_shared<LCDimRadial>(dimRadial);
        _entityContainer.insert(newDimRadial);
        return;
    }

    // Add 'DimDiametric'
    const auto dimDiametric = event.entity<lc::DimDiametric>();

    if (dimDiametric != nullptr) {
        auto newDimDiametric = std::make_shared<LCDimDiametric>(dimDiametric);
        _entityContainer.insert(newDimDiametric);
        return;
    }
}

void DocumentCanvas::on_removeEntityEvent(const lc::RemoveEntityEvent& event) {
    auto i = _entityContainer.entityByID(event.entity()->id());
    _entityContainer.remove(i);
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
            di->selected(true);
        });
    } else {
        _entityContainer.entitiesWithinAndCrossingArea(*_selectedArea).each< LCVDrawItem >([](LCVDrawItem_SPtr di) {
            di->selected(true);
        });

    }
}

void DocumentCanvas::makeSelectionDevice(unsigned int x, unsigned int y, unsigned int w, unsigned int h, bool occupies, bool addTo) {
    LcPainter* painter = cachedPainter(VIEWER_DOCUMENT);
    // Find mouse position in user space
    double dx = x;
    double dy = y;
    double dw = w;
    double dh = h;

    // Calculate to device space
    painter->device_to_user(&dx, &dy);
    painter->device_to_user_distance(&dw, &dh);

    // Set that as the selection of items
    makeSelection(dx, dy, dw, dh, occupies, addTo);
}

void DocumentCanvas::removeSelectionArea() {
    if (_selectedArea != nullptr) {
        delete _selectedArea;
        _selectedArea = nullptr;
    }
}

void DocumentCanvas::removeSelection() {
    _entityContainer.each< LCVDrawItem >([](LCVDrawItem_SPtr di) {
        di->selected(false);
    });
    removeSelectionArea();
}
