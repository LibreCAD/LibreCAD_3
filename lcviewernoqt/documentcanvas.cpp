#include "documentcanvas.h"
#include <cad/meta/metacolor.h>
#include <cad/storage/document.h>
#include <cad/storage/quadtree.h>
#include <cad/geometry/geoarea.h>
#include <cad/primitive/line.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/ellipse.h>
#include <cad/primitive/text.h>
#include <cad/primitive/image.h>
#include <cad/primitive/insert.h>
#include "lcdrawoptions.h"
#include "drawitems/lcvcircle.h"
#include "drawitems/lcvhatch.h"
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
#include <cmath>

#include <typeinfo>

using namespace lc;
using namespace lc::viewer;

DocumentCanvas::DocumentCanvas(const std::shared_ptr<lc::storage::Document>& document, std::function<void(double*, double*)> deviceToUser, meta::Block_CSPtr viewport) :
        _document(document),
        _zoomMin(0.005),
        _zoomMax(200.0),
        _deviceWidth(0),
        _deviceHeight(0),
        _selectedArea(nullptr),
        _selectedAreaIntersects(false),
        _deviceToUser(std::move(deviceToUser)),
        _painterPtr(nullptr),
        _viewport(viewport)
{
    document->addEntityEvent().connect<DocumentCanvas, &DocumentCanvas::on_addEntityEvent>(this);
    document->removeEntityEvent().connect<DocumentCanvas, &DocumentCanvas::on_removeEntityEvent>(this);
    document->commitProcessEvent().connect<DocumentCanvas, &DocumentCanvas::on_commitProcessEvent>(this);

    // Render code for selected area
    _selectedAreaPainter = [](LcPainter & painter, lc::geo::Area area , bool occupies) {
        double dashes[] = {10.0, 3.0, 3.0, 3.0};
        painter.save();
        painter.disable_antialias();
        painter.line_width(2.0);

        if (occupies) {
            painter.source_rgba(.2, .2, 1.0, .6);
        } 
        else {
            painter.source_rgba(.2, 1.0, .2, .5);
        }

        painter.rectangle(area.minP().x(), area.minP().y(), area.width(), area.height());
        painter.fill();
        painter.stroke();

        
        painter.rectangle(area.minP().x(), area.minP().y(), area.width(), area.height());

        if (occupies) {
            painter.source_rgba(.2, .2, 1., 0.9);
        } 
        else {
            painter.source_rgba(.2, 1.0, .2, 0.8);
        }
         
        painter.set_dash(dashes, 4, 0, true);
        painter.stroke();
        painter.dash_destroy();
        painter.restore();
    }; 
}

DocumentCanvas::~DocumentCanvas() {
    _document->addEntityEvent().disconnect<DocumentCanvas, &DocumentCanvas::on_addEntityEvent>(this);
    _document->removeEntityEvent().disconnect<DocumentCanvas, &DocumentCanvas::on_removeEntityEvent>(this);
    _document->commitProcessEvent().disconnect<DocumentCanvas, &DocumentCanvas::on_commitProcessEvent>(this);

    if (_selectedArea != nullptr) {
        delete _selectedArea;
        _selectedArea = nullptr;
    }
}

void DocumentCanvas::setPainter(LcPainter* painter)
{  
    _painterPtr=painter;
}

/*
 * Taking pan as relative
 */

void DocumentCanvas::pan(LcPainter& painter, double move_x, double move_y) {
    double tX = 0;
    double tY = 0;
    painter.device_to_user(&tX,&tY);
    painter.device_to_user(&move_x,&move_y);
    painter.translate(move_x-tX, -move_y+tY);
}

void DocumentCanvas::zoom(LcPainter& painter, double factor, bool relativezoom,
                          unsigned int deviceCenterX, unsigned int deviceCenterY) {
    // Test for minimum and maximum zoom levels
    if ((_zoomMax <= painter.scale() && factor > 1.) || (_zoomMin >= painter.scale() && factor < 1.)) {
        return;
    }
    
    // Find user location at the device location
    painter.save();
    double userX = deviceCenterX;
    double userY = deviceCenterY;
    painter.device_to_user(&userX, &userY);
    painter.restore();

    zoom(painter, factor, relativezoom, userX, userY, deviceCenterX, deviceCenterY);
}

void DocumentCanvas::zoom(LcPainter& painter, double factor, bool relativezoom,
                          double userCenterX, double userCenterY,
                          unsigned int deviceCenterX, unsigned int deviceCenterY) {
    if ((_zoomMax <= painter.scale() && factor > 1.) || (_zoomMin >= painter.scale() && factor < 1.)) {
        return;
    }

    if (relativezoom) {
        factor = factor * painter.scale();
    }
    

    // Calculate reference device offset at device location
    painter.save();
    double refX = deviceCenterX;
    double refY = deviceCenterY;
    painter.reset_transformations();
    painter.scale(factor);
    painter.device_to_user(&refX, &refY);
    painter.restore();

    painter.reset_transformations();
    painter.scale(factor);       
    painter.translate(refX - userCenterX,-refY + userCenterY);
}

void DocumentCanvas::autoScale(LcPainter& painter) {
    auto extends = entityContainer().boundingBox();
    extends = extends.increaseBy(std::min(extends.width(), extends.height()) * 0.1);

    setDisplayArea(painter, extends);
}

void DocumentCanvas::setDisplayArea(LcPainter& painter, const lc::geo::Area& area) {
    double zoom = std::min(_deviceWidth / area.width(), _deviceHeight / area.height());
    this->zoom(painter, zoom, false,
               area.width() / 2 + area.minP().x(),
               area.height() / 2. + area.minP().y(),
               (unsigned int) (_deviceWidth / 2.),
               (unsigned int) (_deviceHeight / 2.)
    );
}

void DocumentCanvas::render(LcPainter& painter, PainterType type) {
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

   
    LcDrawOptions lcDrawOptions;
    event::DrawEvent drawEvent(painter, lcDrawOptions, visibleUserArea);

    switch(type) {
        case VIEWER_BACKGROUND: {
            painter.clear(0.133,0.545,0.133); 
            painter.lineWidthCompensation(0.);
            _background(drawEvent);
            break;
        }

        case VIEWER_DOCUMENT: {
            // Draw Document
            // caller is responsible for clearing    painter.clear(1., 1., 1., 0.);
            painter.source_rgb(1., 1., 1.);
            painter.lineWidthCompensation(0.5);
            painter.enable_antialias();
            auto visibleEntities = entityContainer().entitiesWithinAndCrossingAreaFast(visibleUserArea);
            std::vector<lc::viewer::LCVDrawItem_SPtr> visibleDrawables;
            visibleEntities.each< const lc::entity::CADEntity >([&](lc::entity::CADEntity_CSPtr entity) {
                auto di = _entityDrawItem[entity->id()];
                if(di){
                    visibleDrawables.push_back(di);
                }
            });
            for(const auto& di: visibleDrawables) {
                if(painter.isCachingEnabled() && di->cacheable())
                {    
                    if(painter.isEntityCached( (di->entity())->id() ) == true)
                    {  
                        drawCachedEntity(painter,di);
                    }
                    else
                    {
	                cacheEntity((di->entity())->id(), di);
                        drawCachedEntity(painter, di);
                    }
                }
                else
                {
                    drawEntity(painter, di);
                }
            };
            painter.line_width(1.);
            painter.source_rgb(1., 1., 1.);
            painter.lineWidthCompensation(0.); 
            break;
        }

        case VIEWER_FOREGROUND: {
            _foreground(drawEvent);
            // Draw selection rectangle
            if (_selectedArea != nullptr) {
                _selectedAreaPainter(painter, *_selectedArea, _selectedAreaIntersects);
            }
            break;
        }
    }
}

double DocumentCanvas::drawWidth(const lc::entity::CADEntity_CSPtr& entity, const lc::entity::Insert_CSPtr& insert) {
    auto entityLineWidth = entity->metaInfo<lc::meta::MetaLineWidth>(lc::meta::MetaLineWidthByValue::LCMETANAME());
    auto entityLineWidthByValue = std::dynamic_pointer_cast<const lc::meta::MetaLineWidthByValue>(entityLineWidth);

    if (entityLineWidthByValue != nullptr) {
        return entityLineWidthByValue->width();
    }
    else if(insert != nullptr &&
            std::dynamic_pointer_cast<const lc::meta::MetaLineWidthByBlock>(entityLineWidth) != nullptr) {
        auto insertLW = insert->metaInfo<lc::meta::MetaLineWidthByValue>(lc::meta::MetaLineWidth::LCMETANAME());

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
        const lc::entity::CADEntity_CSPtr& entity,
        const lc::entity::Insert_CSPtr& insert,
        double width) {
    auto layer = entity->layer();

    lc::meta::DxfLinePattern_CSPtr entityLinePattern = entity->metaInfo<lc::meta::DxfLinePattern>(lc::meta::DxfLinePattern::LCMETANAME());
    auto linePatternByValue = std::dynamic_pointer_cast<const lc::meta::DxfLinePatternByValue>(entityLinePattern);
    auto linePatternByBlock = std::dynamic_pointer_cast<const lc::meta::DxfLinePatternByBlock>(entityLinePattern);

    if (linePatternByValue != nullptr && !(linePatternByValue->lcPattern(width).empty())) {
        return linePatternByValue->lcPattern(width);
    }
    else if(linePatternByBlock != nullptr && insert != nullptr) {
        auto insertLP = insert->metaInfo<lc::meta::DxfLinePatternByValue>(lc::meta::DxfLinePattern::LCMETANAME());

        if(insertLP != nullptr) {
            return insertLP->lcPattern(width);
        }
        else if(insert->layer()->linePattern() != nullptr) {
            return insert->layer()->linePattern()->lcPattern(width);
        }
    }
    else if(layer->linePattern() != nullptr && !(layer->linePattern()->lcPattern(width).empty())) {
        return layer->linePattern()->lcPattern(width);
    }

	return std::vector<double>();
}

lc::Color DocumentCanvas::drawColor(const lc::entity::CADEntity_CSPtr& entity, const lc::entity::Insert_CSPtr& insert,
                                    bool selected) {
    LcDrawOptions lcDrawOptions;

    lc::meta::MetaColor_CSPtr entityColor = entity->metaInfo<lc::meta::MetaColor>(lc::meta::MetaColor::LCMETANAME());
    lc::meta::MetaColorByValue_CSPtr colorByValue = std::dynamic_pointer_cast<const lc::meta::MetaColorByValue>(entityColor);

    if (selected) {
        return lcDrawOptions.selectedColor();
    }
    else if (colorByValue != nullptr) {
        return colorByValue->color();
    }
    else if(insert != nullptr &&
            std::dynamic_pointer_cast<const lc::meta::MetaColorByBlock>(entityColor) != nullptr) {
        auto insertColor = insert->metaInfo<lc::meta::MetaColorByValue>(lc::meta::MetaColor::LCMETANAME());

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

void DocumentCanvas::drawEntity(LcPainter& painter, const LCVDrawItem_CSPtr& drawable,
                                const lc::entity::Insert_CSPtr& insert) {
    LcDrawOptions lcDrawOptions;

    double x = 0.;
    double y = 0.;
    double w = _deviceWidth;
    double h = _deviceHeight;
    painter.device_to_user(&x, &y);
    painter.device_to_user_distance(&w, &h);
    lc::geo::Area visibleUserArea = lc::geo::Area(lc::geo::Coordinate(x, y), w, h);

    auto asInsert = std::dynamic_pointer_cast<const LCVInsert>(drawable);
    if(asInsert != nullptr) {
        asInsert->draw(shared_from_this(), painter);
        return;
    }

    painter.save();

    lc::entity::CADEntity_CSPtr ci = drawable->entity();


    // Used to give the illusation from slightly thinner lines. Not sure yet what to d with it and if I will keep it
    double alpha_compensation = 0.9;

    // Decide on line width
    // We multiply for now by 3 to ensure that 1mm lines will still appear thicker on screen
    // TODO: Find a better algo
    double width = drawWidth(ci, insert) * 1.5;

    // Is this correct? May be we should decide on a different minimum width then 0.1, because may be on some devices 0.11 isn't visible?
    painter.line_width(std::max(width, MINIMUM_READER_LINEWIDTH));

    auto path = drawLinePattern(ci, insert, width);
    painter.set_dash(path.data(), path.size(), 0., true);

    // Decide what color to render the entity into

    auto color = drawColor(ci, insert, drawable->selected());
    painter.source_rgba(
            color.red(),
            color.green(),
            color.blue(),
            color.alpha() * alpha_compensation
    );
  
   
    drawable->draw(painter, lcDrawOptions, visibleUserArea);

    painter.restore();
    painter.dash_destroy();  
}

void DocumentCanvas::drawCachedEntity(LcPainter& painter, const LCVDrawItem_CSPtr& drawable,
                                const lc::entity::Insert_CSPtr& insert) {
    LcDrawOptions lcDrawOptions;

    double wc = _deviceWidth;
    double hc = _deviceHeight;
    double xc = 0.;
    double yc = 0.;
    painter.device_to_user(&xc, &yc);
    painter.device_to_user_distance(&wc, &hc);
    lc::geo::Area visibleUserArea = lc::geo::Area(lc::geo::Coordinate(xc, yc), wc, hc);

    auto asInsert = std::dynamic_pointer_cast<const LCVInsert>(drawable);
    if(asInsert != nullptr) 
    {
        asInsert->draw(shared_from_this(), painter);
        return;
    }
    // getting CADEntity_CSPtr
    lc::entity::CADEntity_CSPtr ce = drawable->entity();
    double alpha_compensation = 0.9;
    painter.save();
    // Decide what color to render the entity into
    auto color = drawColor(ce, insert, drawable->selected());
    painter.source_rgba(
            color.red(),
            color.green(),
            color.blue(),
            color.alpha() * alpha_compensation
    );
  
    painter.renderEntityCached( (drawable->entity())->id() );
    painter.restore();  
}

void DocumentCanvas::cacheEntity(unsigned long id, const LCVDrawItem_CSPtr& drawable,
                                const lc::entity::Insert_CSPtr& insert) {
    LcDrawOptions lcDrawOptions;
    double x = 0.;
    double y = 0.;
    double w = _deviceWidth;
    double h = _deviceHeight;
    (*_painterPtr).device_to_user(&x, &y);
    (*_painterPtr).device_to_user_distance(&w, &h);
    lc::geo::Area visibleUserArea = lc::geo::Area(lc::geo::Coordinate(x, y), w, h);

    auto asInsert = std::dynamic_pointer_cast<const LCVInsert>(drawable);
    if(asInsert != nullptr) {
        asInsert->draw(shared_from_this(), (*_painterPtr));
        return;
    }

    //=======PICK the caching painter instance=============
          LcPainter* cachepainter=(*_painterPtr).getCacherpainter();
    //=====================================================
     cachepainter->startcaching();
    cachepainter->save();

    lc::entity::CADEntity_CSPtr ci = drawable->entity();


    // Used to give the illusation from slightly thinner lines. Not sure yet what to d with it and if I will keep it
    double alpha_compensation = 0.9;

    // Decide on line width
    // We multiply for now by 3 to ensure that 1mm lines will still appear thicker on screen
    // TODO: Find a better algo
    double width = drawWidth(ci, insert) * 1.5;

    // Is this correct? May be we should decide on a different minimum width then 0.1, because may be on some devices 0.11 isn't visible?
    cachepainter->line_width(std::max(width, MINIMUM_READER_LINEWIDTH));

    auto path = drawLinePattern(ci, insert, width);
    cachepainter->set_dash(path.data(), path.size(), 0., true);

    // Decide what color to render the entity into

    auto color = drawColor(ci, insert, drawable->selected());
    cachepainter->source_rgba(
            color.red(),
            color.green(),
            color.blue(),
            color.alpha() * alpha_compensation
    );

    //===========Here caching happens==============
    drawable->draw( (*cachepainter), lcDrawOptions, visibleUserArea);
    cachepainter->finishcaching(id);
    cachepainter->restore();  
    cachepainter->dash_destroy();
    //===============================================
}

void DocumentCanvas::on_commitProcessEvent(const lc::event::CommitProcessEvent& event) {
    entityContainer().optimise();
}

// This assumes that the entity has already been added to _document->entityContainer()
void DocumentCanvas::on_addEntityEvent(const lc::event::AddEntityEvent& event) {
    auto entity = event.entity();
    auto drawable = asDrawable(entity);
    _entityDrawItem.insert(std::make_pair(entity->id(), drawable));
}

void DocumentCanvas::on_removeEntityEvent(const lc::event::RemoveEntityEvent& event) { 
     entityContainer().remove(event.entity());
    _entityDrawItem.erase((event.entity())->id());
    if(_painterPtr!=NULL && ((*_painterPtr).isCachingEnabled()) )
    (*_painterPtr).deleteEntityCached( (event.entity())->id() );  // Delete the cacahed pack
}

std::shared_ptr<lc::storage::Document> DocumentCanvas::document() const {
    return _document;
}

lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr> DocumentCanvas::entityContainer() const {
    return _document->entitiesByBlock(_viewport);
}

lc::geo::Area DocumentCanvas::bounds() const {
    return entityContainer().bounds();
}

void DocumentCanvas::makeSelection(double x, double y, double w, double h, bool occupies) {
    if (_selectedArea != nullptr) {
        delete _selectedArea;
        _selectedArea = nullptr;
    }

    _selectedArea = new lc::geo::Area(lc::geo::Coordinate(x, y), lc::geo::Coordinate(x + w, y + h));
    // std::cout << *_selectedArea << std::endl;
    _selectedAreaIntersects = occupies;

    // remove old new selection
    for(const auto& di: _newSelection) {
        di->selected(false);
    }
    _newSelection.clear();

    // Refresh: old new selection has been canceled
    for(const auto& di: _selectedDrawables) {
        di->selected(true);
    }

    lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr> entitiesInSelection;
    if (occupies) {
        entitiesInSelection = entityContainer().entitiesFullWithinArea(*_selectedArea);
    }
    else {
        entitiesInSelection = entityContainer().entitiesWithinAndCrossingArea(*_selectedArea);
    }
    entitiesInSelection.each< const lc::entity::CADEntity >([&](lc::entity::CADEntity_CSPtr entity) {
        auto di = _entityDrawItem[entity->id()];
	// add if it does not previously exist
        auto iter = std::find(_newSelection.begin(), _newSelection.end(), di);
        if(iter == _newSelection.end())
		_newSelection.push_back(di);// indicate needs update
        di->selected(!di->selected());
    });
}

lc::viewer::LCVDrawItem_SPtr DocumentCanvas::getDrawable(const lc::entity::CADEntity_CSPtr& entity) {
    return _entityDrawItem[entity->id()];
}

void DocumentCanvas::makeSelectionDevice(LcPainter& painter, unsigned int x, unsigned int y, unsigned int w, unsigned int h, bool occupies) {
    // Find mouse position in user space
    double dx = x;
    double dy = y;
    double dw = w;
    double dh = h;

    // Calculate to device space
    painter.device_to_user(&dx, &dy);
    painter.device_to_user_distance(&dw, &dh);

    // Set that as the selection of items
    makeSelection(dx, dy, dw, dh, occupies);
}

void DocumentCanvas::closeSelection() {
    for(const auto& drawable: _newSelection) {
        auto iter = std::find(_selectedDrawables.begin(), _selectedDrawables.end(), drawable);
        if(iter == _selectedDrawables.end()) {
	    if(drawable->selected())
	            _selectedDrawables.push_back(drawable);
        }else{
	    if(!drawable->selected())
	            _selectedDrawables.erase(iter);
        }
    };

    _newSelection.clear();
    // Refresh
    for(const auto& di: _selectedDrawables) {
        di->selected(true);
    }
}

void DocumentCanvas::removeSelectionArea() {
    if (_selectedArea != nullptr) {
        delete _selectedArea;
        _selectedArea = nullptr;
    }
}

void DocumentCanvas::selectAll() {
    _selectedDrawables.clear();
    _newSelection.clear();

    entityContainer().each< const lc::entity::CADEntity >([&](lc::entity::CADEntity_CSPtr entity) {
        _entityDrawItem[entity->id()]->selected(true);
        _selectedDrawables.push_back(_entityDrawItem[entity->id()]);
    });
}

void DocumentCanvas::removeSelection() {
    for(const auto& di: _selectedDrawables) {
        di->selected(false);
    };

    _selectedDrawables.clear();
}

void DocumentCanvas::inverseSelection() {
    _selectedDrawables.clear();
    _newSelection.clear();

    entityContainer().each< const lc::entity::CADEntity >([&](lc::entity::CADEntity_CSPtr entity) {
        lc::viewer::LCVDrawItem_SPtr item = _entityDrawItem[entity->id()];
        auto iter = std::find(_selectedDrawables.begin(), _selectedDrawables.end(), item);
        if (iter != _selectedDrawables.end())
        {
            item->selected(false);
            _selectedDrawables.erase(iter);
        }
        else
        {
            item->selected(true);
            _selectedDrawables.push_back(item);
        }
    });
}

Nano::Signal<void(lc::viewer::event::DrawEvent const & event)> & DocumentCanvas::background ()  {
    return _background;
}
Nano::Signal<void(lc::viewer::event::DrawEvent const & event)> & DocumentCanvas::foreground ()  {
    return _foreground;
}

LCVDrawItem_SPtr DocumentCanvas::asDrawable(const lc::entity::CADEntity_CSPtr& entity) {
    // Add a line
    auto line = std::dynamic_pointer_cast<const lc::entity::Line>(entity);

    if (line != nullptr) {
        return std::make_shared<LCVLine>(line);
    }

    // Add a hatch
    auto hatch = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity);

    if (hatch != nullptr) {
        return std::make_shared<LCVHatch>(hatch);
    }

    // Add a circle
    auto circle = std::dynamic_pointer_cast<const lc::entity::Circle>(entity);

    if (circle != nullptr) {
        return std::make_shared<LCVCircle>(circle);
    }

    // Add a Arc
    auto arc = std::dynamic_pointer_cast<const lc::entity::Arc>(entity);

    if (arc != nullptr) {
        return std::make_shared<LCVArc>(arc);
    }


    // Add Ellipse
    auto ellipse = std::dynamic_pointer_cast<const lc::entity::Ellipse>(entity);

    if (ellipse != nullptr) {
        return std::make_shared<LCVEllipse>(ellipse);
    }

    // Add Text
    auto text = std::dynamic_pointer_cast<const lc::entity::Text>(entity);

    if (text != nullptr) {
        return std::make_shared<LCVText>(text);
    }

    // Add 'Point' or 'Coordinate'
    auto coord = std::dynamic_pointer_cast<const lc::entity::Point>(entity);

    // Point cannot be cached since it change size(constant size)
    if (coord != nullptr) {
        auto di = std::make_shared<LCVPoint>(coord);
	di->cacheable(false);
	return di;
    }

    // Add 'DimRadial'
    auto dimRadial = std::dynamic_pointer_cast<const lc::entity::DimRadial>(entity);

    if (dimRadial != nullptr) {
        return std::make_shared<LCDimRadial>(dimRadial);
    }

    // Add 'DimDiametric'
    auto dimDiametric = std::dynamic_pointer_cast<const lc::entity::DimDiametric>(entity);

    if (dimDiametric != nullptr) {
        return std::make_shared<LCDimDiametric>(dimDiametric);
    }

    // Add 'DimLinear'
    auto dimLinear = std::dynamic_pointer_cast<const lc::entity::DimLinear>(entity);

    if (dimLinear != nullptr) {
        return std::make_shared<LCDimLinear>(dimLinear);
    }

    // Add 'DimAligned'
    auto dimAligned = std::dynamic_pointer_cast<const lc::entity::DimAligned>(entity);

    if (dimAligned != nullptr) {
        return std::make_shared<LCDimAligned>(dimAligned);
    }

    // Add 'DimAngular'
    auto dimAngular = std::dynamic_pointer_cast<const lc::entity::DimAngular>(entity);

    if (dimAngular != nullptr) {
        return std::make_shared<LCDimAngular>(dimAngular);
    }

    // Add 'LWPolyline'
    auto lwPolyline = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(entity);

    if (lwPolyline != nullptr) {
        return std::make_shared<LCLWPolyline>(lwPolyline);
    }

    // Add 'Spline'
    auto spline = std::dynamic_pointer_cast<const lc::entity::Spline>(entity);

    if (spline != nullptr) {
        return std::make_shared<LCVSpline>(spline);
    }

    // Add 'Image'
    auto image = std::dynamic_pointer_cast<const lc::entity::Image>(entity);

    if (image != nullptr) {
        return std::make_shared<LCImage>(image);
    }

    // Add 'Insert'
    auto insert = std::dynamic_pointer_cast<const lc::entity::Insert>(entity);

    if (insert != nullptr) {
        return std::make_shared<LCVInsert>(insert);
    }

    return nullptr;
}

std::vector<lc::viewer::LCVDrawItem_SPtr>& DocumentCanvas::selectedDrawables() {
    return _selectedDrawables;
}

lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr> DocumentCanvas::selectedEntities() {
    lc::storage::EntityContainer<lc::entity::CADEntity_CSPtr> entitiesInSelection;
    for(const auto& di: _selectedDrawables) {
        entitiesInSelection.insert(di->entity());
    }
    return entitiesInSelection;
}

void DocumentCanvas::newDeviceSize(unsigned int width, unsigned int height) {
    _deviceWidth = width;
    _deviceHeight = height;
}

void DocumentCanvas::selectPoint(double x, double y) {
    double zeroX = 0.;
    double zeroY = 0.;
    double w = 5;
    double h = 5;

    device_to_user(&zeroX, &zeroY);
    device_to_user(&w, &h);
    w = w - zeroX;
    auto point = geo::Coordinate(x,y);
    double mwh = sqrt(2)*w;

    lc::geo::Area selectionArea(lc::geo::Coordinate(x - w, y - w), w * 2, w * 2);
    auto entities = entityContainer().entitiesWithinAndCrossingAreaFast(selectionArea);
    entities.each< const lc::entity::CADEntity >([=](lc::entity::CADEntity_CSPtr entity) {
    	//Check if it is on entity
	auto snapable = std::dynamic_pointer_cast<const lc::entity::Snapable>(entity); 
 
 	if(snapable){
	    	auto nearestPoint = snapable->nearestPointOnEntity(point);
	    	auto distance = nearestPoint.distanceTo(point);
	    	//std::cout << nearestPoint << std::endl;
	    	//std::cout << point << std::endl;
	    	if (distance>mwh)
	    		return;
    	};
	auto di = _entityDrawItem[entity->id()];
	auto iter = std::find(_selectedDrawables.begin(), _selectedDrawables.end(), di);
	//if not found in selected drawables
        if (iter==_selectedDrawables.end()){
		di->selected(true);
		_selectedDrawables.push_back(di);
	}else{
		di->selected(false);
		_selectedDrawables.erase(iter);
	}
    });
}

void DocumentCanvas::selectEntity(lc::entity::CADEntity_CSPtr entityPtr) {
    lc::viewer::LCVDrawItem_SPtr entityDrawable = getDrawable(entityPtr);
    entityDrawable->selected(true);
    _selectedDrawables.push_back(entityDrawable);
}
