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
#include <cad/primitive/insert.h>
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

#include "painters/lcpainter.h"
#include "drawitems/lcvinsert.h"

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
        _selectedArea = nullptr;
    }
}

void DocumentCanvas::removePainters()  {
    for (auto i = _cachedPainters.begin(); i != _cachedPainters.end(); i++) {
        this->_deletePainterFunctor(i->second);
    }
    _cachedPainters.clear();

    if (_selectedArea != nullptr) {
        delete _selectedArea;
        _selectedArea = nullptr;
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
    extends = extends.increaseBy(std::min(extends.width(), extends.height()) * 0.1);

    setDisplayArea(extends);
}

void DocumentCanvas::setDisplayArea(const lc::geo::Area& area) {
    double zoom = std::min(_deviceWidth / area.width(), _deviceHeight / area.height());
    this->zoom(zoom, false,
               area.width() / 2 + area.minP().x(),
               area.height() / 2. + area.minP().y(),
               _deviceWidth / 2.,
               _deviceHeight / 2.
    );
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

    visibleItems.each< LCVDrawItem >([&](LCVDrawItem_CSPtr di) {
		drawEntity(di);
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

double DocumentCanvas::drawWidth(lc::entity::CADEntity_CSPtr entity, lc::entity::Insert_CSPtr insert) {
    auto entityLineWidth = entity->metaInfo<lc::MetaLineWidth>(lc::MetaLineWidthByValue::LCMETANAME());
    auto entityLineWidthByValue = std::dynamic_pointer_cast<const lc::MetaLineWidthByValue>(entityLineWidth);

    if (entityLineWidthByValue != nullptr) {
        return entityLineWidthByValue->width();
    }
    else if(insert != nullptr &&
            std::dynamic_pointer_cast<const lc::MetaLineWidthByBlock>(entityLineWidth) != nullptr) {

        auto insertLW = insert->metaInfo<lc::MetaLineWidthByValue>(lc::MetaLineWidth::LCMETANAME());

        if(insertLW != nullptr) {
            return insertLW->width();
        }
        else {
            return insert->layer()->lineWidth().width();
        }
    }
    else {
        return entity->layer()->lineWidth().width();
    }
}

std::vector<double> DocumentCanvas::drawLinePattern(
        lc::entity::CADEntity_CSPtr entity,
        lc::entity::Insert_CSPtr insert,
        double width) {

    auto layer = entity->layer();

    lc::DxfLinePattern_CSPtr entityLinePattern = entity->metaInfo<lc::DxfLinePattern>(lc::DxfLinePattern::LCMETANAME());
    auto linePatternByValue = std::dynamic_pointer_cast<const lc::DxfLinePatternByValue>(entityLinePattern);
    auto linePatternByBlock = std::dynamic_pointer_cast<const lc::DxfLinePatternByBlock>(entityLinePattern);

    if (linePatternByValue != nullptr && linePatternByValue->lcPattern(width).size()>0) {
        return linePatternByValue->lcPattern(width);
    }
    else if(linePatternByBlock != nullptr && insert != nullptr) {
        auto insertLP = insert->metaInfo<lc::DxfLinePatternByValue>(lc::DxfLinePattern::LCMETANAME());

        if(insertLP != nullptr) {
            return insertLP->lcPattern(width);
        }
        else if(insert->layer()->linePattern() != nullptr) {
            return insert->layer()->linePattern()->lcPattern(width);
        }
    }
    else if(layer->linePattern() != nullptr && layer->linePattern()->lcPattern(width).size() > 0) {
        return layer->linePattern()->lcPattern(width);
    }

    return std::vector<double>();
}

lc::Color DocumentCanvas::drawColor(lc::entity::CADEntity_CSPtr entity, lc::entity::Insert_CSPtr insert, bool selected) {
    LcDrawOptions lcDrawOptions;

    lc::MetaColor_CSPtr entityColor = entity->metaInfo<lc::MetaColor>(lc::MetaColor::LCMETANAME());
    lc::MetaColorByValue_CSPtr colorByValue = std::dynamic_pointer_cast<const lc::MetaColorByValue>(entityColor);

    if (selected) {
        return lcDrawOptions.selectedColor();
    }
    else if (colorByValue != nullptr) {
        return colorByValue->color();
    }
    else if(insert != nullptr &&
            std::dynamic_pointer_cast<const lc::MetaColorByBlock>(entityColor) != nullptr) {
        auto insertColor = insert->metaInfo<lc::MetaColorByValue>(lc::MetaColor::LCMETANAME());

        if(insertColor != nullptr) {
            return insertColor->color();
        }
        else {
            return insert->layer()->color();
        }
    }
    else {
        return entity->layer()->color();
    }
}

void DocumentCanvas::drawEntity(LCVDrawItem_CSPtr entity, lc::entity::Insert_CSPtr insert) {
    LcPainter& painter = cachedPainter(VIEWER_DOCUMENT);
    LcDrawOptions lcDrawOptions;

    double x = 0.;
    double y = 0.;
    double w = _deviceWidth;
    double h = _deviceHeight;
    painter.device_to_user(&x, &y);
    painter.device_to_user_distance(&w, &h);
    lc::geo::Area visibleUserArea = lc::geo::Area(lc::geo::Coordinate(x, y), w, h);

    auto asInsert = std::dynamic_pointer_cast<const LCVInsert>(entity);
    if(asInsert != nullptr) {
        asInsert->draw(shared_from_this());
        return;
    }

    painter.save();

	lc::entity::CADEntity_CSPtr ci = std::dynamic_pointer_cast<const lc::entity::CADEntity>(entity);


	// Used to give the illusation from slightly thinner lines. Not sure yet what to d with it and if I will keep it
	double alpha_compensation = 0.9;

    // Decide on line width
    // We multiply for now by 3 to ensure that 1mm lines will still appear thicker on screen
    // TODO: Find a better algo
    double width = drawWidth(ci, insert) * 1.5;

    // Is this correct? May be we should decide on a different minimum width then 0.1, because may be on some devices 0.11 isn't visible?
    painter.line_width(std::max(width, MINIMUM_READER_LINEWIDTH));

    auto path = drawLinePattern(ci, insert, width);
    painter.set_dash(&path[0], path.size(), 0., true);

	// Decide what color to render the entity into

    auto color = drawColor(ci, insert, entity->selected());
    painter.source_rgba(
            color.red(),
            color.green(),
            color.blue(),
            color.alpha() * alpha_compensation
    );

	entity->draw(painter, lcDrawOptions, visibleUserArea);

	painter.restore();	
}

void DocumentCanvas::on_commitProcessEvent(const lc::CommitProcessEvent&) {
    _entityContainer.optimise();
}

void DocumentCanvas::on_addEntityEvent(const lc::AddEntityEvent& event) {
    auto entity = event.entity();

    if(entity->block() != nullptr) {
        return;
    }

    auto drawable = asDrawable(event.entity());

    if (drawable != nullptr) {
		auto drawableEntity = std::dynamic_pointer_cast<lc::entity::CADEntity>(drawable);
        _entityContainer.insert(drawableEntity);
    }
}

void DocumentCanvas::on_removeEntityEvent(const lc::RemoveEntityEvent& event) {
    auto i = _entityContainer.entityByID(event.entity()->id());

    _entityContainer.remove(i);
}

std::shared_ptr<lc::Document> DocumentCanvas::document() const {
    return _document;
}

const lc::EntityContainer<lc::entity::CADEntity_SPtr>& DocumentCanvas::entityContainer() const {
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
        _selectedArea = nullptr;
    }

    _selectedArea = new lc::geo::Area(lc::geo::Coordinate(x, y), lc::geo::Coordinate(x + w, y + h));
    // std::cout << *_selectedArea << std::endl;
    _selectedAreaIntersects = occupies;

    // Remove current selection
    if (!addTo) {
        removeSelection();
    }

    _newSelection.each< LCVDrawItem >([](LCVDrawItem_SPtr di) {
        di->selected(false);
    });

    _selectedEntities.each< LCVDrawItem >([](LCVDrawItem_SPtr di) {
        di->selected(true);
    });

    if (occupies) {
        _newSelection = _entityContainer.entitiesFullWithinArea(*_selectedArea);
    } else {
        _newSelection = _entityContainer.entitiesWithinAndCrossingArea(*_selectedArea);
    }



    _newSelection.each< LCVDrawItem >([&](LCVDrawItem_SPtr di) {
        // std::cerr<< __FILE__ << " : " << __FUNCTION__ << " : " << __LINE__ << " " << typeid(*di).name() << std::endl;
        auto entity = std::dynamic_pointer_cast<lc::entity::CADEntity>(di);
        if(entity && _selectedEntities.entityByID(entity->id()) != nullptr) {
            di->selected(false);
        }
        else {
            di->selected(true);
        }
    });
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

void DocumentCanvas::closeSelection() {
    _newSelection.each<lc::entity::CADEntity>([&](lc::entity::CADEntity_SPtr entity) {
        auto drawable = std::dynamic_pointer_cast<LCVDrawItem>(entity);

        if(_selectedEntities.entityByID(entity->id()) != nullptr) {
            _selectedEntities.remove(entity);
            drawable->selected(false);
        }
        else {
            _selectedEntities.insert(entity);
            drawable->selected(true);
        }
    });

    _newSelection = lc::EntityContainer<lc::entity::CADEntity_SPtr>();
}

void DocumentCanvas::removeSelectionArea() {
    if (_selectedArea != nullptr) {
        delete _selectedArea;
        _selectedArea = nullptr;
    }
}

void DocumentCanvas::removeSelection() {
    _selectedEntities.each< LCVDrawItem >([](LCVDrawItem_SPtr di) {
        di->selected(false);
    });

    _selectedEntities = lc::EntityContainer<lc::entity::CADEntity_SPtr>();
}

Nano::Signal<void(DrawEvent const & event)> & DocumentCanvas::background ()  {
    return _background;
}
Nano::Signal<void(DrawEvent const & event)> & DocumentCanvas::foreground ()  {
    return _foreground;
}

LCVDrawItem_SPtr DocumentCanvas::asDrawable(lc::entity::CADEntity_CSPtr entity) {
    // Add a line
    const auto line = std::dynamic_pointer_cast<const lc::entity::Line>(entity);

    if (line != nullptr) {
        return std::make_shared<LCVLine>(line);
    }

    // Add a circle
    const auto circle = std::dynamic_pointer_cast<const lc::entity::Circle>(entity);

    if (circle != nullptr) {
        return std::make_shared<LCVCircle>(circle);
    }

    // Add a Arc
    const auto arc = std::dynamic_pointer_cast<const lc::entity::Arc>(entity);

    if (arc != nullptr) {
        return std::make_shared<LCVArc>(arc);
    }


    // Add Ellipse
    const auto ellipse = std::dynamic_pointer_cast<const lc::entity::Ellipse>(entity);

    if (ellipse != nullptr) {
        return std::make_shared<LCVEllipse>(ellipse);
    }

    // Add Text
    const auto text = std::dynamic_pointer_cast<const lc::entity::Text>(entity);

    if (text != nullptr) {
        return std::make_shared<LCVText>(text);
    }

    // Add 'Point' or 'Coordinate'
    const auto coord = std::dynamic_pointer_cast<const lc::entity::Point>(entity);

    if (coord != nullptr) {
        return std::make_shared<LCVPoint>(coord);
    }

    // Add 'DimRadial'
    const auto dimRadial = std::dynamic_pointer_cast<const lc::entity::DimRadial>(entity);

    if (dimRadial != nullptr) {
        return std::make_shared<LCDimRadial>(dimRadial);
    }

    // Add 'DimDiametric'
    const auto dimDiametric = std::dynamic_pointer_cast<const lc::entity::DimDiametric>(entity);

    if (dimDiametric != nullptr) {
        return std::make_shared<LCDimDiametric>(dimDiametric);
    }

    // Add 'DimLinear'
    const auto dimLinear = std::dynamic_pointer_cast<const lc::entity::DimLinear>(entity);

    if (dimLinear != nullptr) {
        return std::make_shared<LCDimLinear>(dimLinear);
    }

    // Add 'DimAligned'
    const auto dimAligned = std::dynamic_pointer_cast<const lc::entity::DimAligned>(entity);

    if (dimAligned != nullptr) {
        return std::make_shared<LCDimAligned>(dimAligned);
    }

    // Add 'DimAngular'
    const auto dimAngular = std::dynamic_pointer_cast<const lc::entity::DimAngular>(entity);

    if (dimAngular != nullptr) {
        return std::make_shared<LCDimAngular>(dimAngular);
    }

    // Add 'LWPolyline'
    const auto lwPolyline = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(entity);

    if (lwPolyline != nullptr) {
        return std::make_shared<LCLWPolyline>(lwPolyline);
    }

    // Add 'Spline'
    const auto spline = std::dynamic_pointer_cast<const lc::entity::Spline>(entity);

    if (spline != nullptr) {
        return std::make_shared<LCVSpline>(spline);
    }

    // Add 'Image'
    const auto image = std::dynamic_pointer_cast<const lc::entity::Image>(entity);

    if (image != nullptr) {
        return std::make_shared<LCImage>(image);
    }

    // Add 'Insert'
    const auto insert = std::dynamic_pointer_cast<const lc::entity::Insert>(entity);

    if (insert != nullptr) {
        return std::make_shared<LCVInsert>(insert);
    }

    return nullptr;
}

lc::EntityContainer<lc::entity::CADEntity_SPtr> DocumentCanvas::selection() {
    return _selectedEntities;
}
