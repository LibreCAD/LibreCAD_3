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
#include <cmath>

#include <typeinfo>

using namespace LCViewer;

DocumentCanvas::DocumentCanvas(const std::shared_ptr<lc::Document>& document, std::function<void(double*, double*)> deviceToUser) :
        _document(document),
        _zoomMin(0.005),
        _zoomMax(200.0),
        _deviceWidth(0),
        _deviceHeight(0),
        _selectedArea(nullptr),
        _selectedAreaIntersects(false),
        _deviceToUser(std::move(deviceToUser)) {


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
        } 
        else {
            painter.source_rgba(.2, 1.0, .2, .5);
        }

        painter.rectangle(area.minP().x(), area.minP().y(), area.width(), area.height());
        painter.fill();
        painter.rectangle(area.minP().x(), area.minP().y(), area.width(), area.height());

        if (occupies) {
            painter.source_rgba(.2, .2, 1., 0.9);
        } 
        else {
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

    if (_selectedArea != nullptr) {
        delete _selectedArea;
        _selectedArea = nullptr;
    }
}

/*
 * TODO: Code needs to be Tested.
 */

void DocumentCanvas::pan(LcPainter& painter, double move_x, double move_y) {
    double pan_x=0.;
    double pan_y=0.;
    painter.getTranslate(&pan_x, &pan_y);

    /* FIXME 100.0 should be dynamically calculated, depends on the drawing speed */
    if (std::abs(pan_x-move_x) > 100.0 || std::abs(pan_y - move_y) > 100.0 || pan_x == 0.0 || pan_y == 0.0) {
        pan_x = move_x;
        pan_y = move_y;
    }

    painter.translate(move_x - pan_x, move_y - pan_y);
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
    auto extends = _document->entityContainer().boundingBox();
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
    DrawEvent drawEvent(painter, lcDrawOptions, visibleUserArea);

    switch(type) {
        case VIEWER_BACKGROUND: {
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

            auto visibleEntities = _document->entityContainer().entitiesWithinAndCrossingAreaFast(visibleUserArea);
            std::vector<LCViewer::LCVDrawItem_SPtr> visibleDrawables;
            visibleEntities.each< const lc::entity::CADEntity >([&](lc::entity::CADEntity_CSPtr entity) {
                auto di = _entityDrawItem[entity];
                visibleDrawables.push_back(di);
            });

            for(auto di: visibleDrawables) {
                drawEntity(painter, di);
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
        const lc::entity::CADEntity_CSPtr& entity,
        const lc::entity::Insert_CSPtr& insert,
        double width) {

    auto layer = entity->layer();

    lc::DxfLinePattern_CSPtr entityLinePattern = entity->metaInfo<lc::DxfLinePattern>(lc::DxfLinePattern::LCMETANAME());
    auto linePatternByValue = std::dynamic_pointer_cast<const lc::DxfLinePatternByValue>(entityLinePattern);
    auto linePatternByBlock = std::dynamic_pointer_cast<const lc::DxfLinePatternByBlock>(entityLinePattern);

    if (linePatternByValue != nullptr && !(linePatternByValue->lcPattern(width).empty())) {
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
    else if(layer->linePattern() != nullptr && !(layer->linePattern()->lcPattern(width).empty())) {
        return layer->linePattern()->lcPattern(width);
    }

    return std::vector<double>();
}

lc::Color DocumentCanvas::drawColor(const lc::entity::CADEntity_CSPtr& entity, const lc::entity::Insert_CSPtr& insert,
                                    bool selected) {
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
    painter.set_dash(&path[0], path.size(), 0., true);

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
}

void DocumentCanvas::on_commitProcessEvent(const lc::CommitProcessEvent& event) {
    _document->entityContainer().optimise();
}

// This assumes that the entity has already been added to _document->entityContainer()
void DocumentCanvas::on_addEntityEvent(const lc::AddEntityEvent& event) {
    auto entity = event.entity();

    if(entity->block() != nullptr) {
        return;
    }

    auto drawable = asDrawable(event.entity());

    if (drawable != nullptr) {
        auto drawableEntity = std::dynamic_pointer_cast<LCViewer::LCVDrawItem>(drawable);
        _entityDrawItem.insert(std::make_pair(drawableEntity->entity(), drawableEntity));
    }
}

void DocumentCanvas::on_removeEntityEvent(const lc::RemoveEntityEvent& event) {
    _document->entityContainer().remove(event.entity());
    _entityDrawItem.erase(event.entity());
}

std::shared_ptr<lc::Document> DocumentCanvas::document() const {
    return _document;
}

lc::EntityContainer<lc::entity::CADEntity_CSPtr>& DocumentCanvas::entityContainer() const {
    return _document->entityContainer();
}

lc::geo::Area DocumentCanvas::bounds() const {
    return _document->entityContainer().bounds();
}

void DocumentCanvas::makeSelection(double x, double y, double w, double h, bool occupies) {
    if (_selectedArea != nullptr) {
        delete _selectedArea;
        _selectedArea = nullptr;
    }

    _selectedArea = new lc::geo::Area(lc::geo::Coordinate(x, y), lc::geo::Coordinate(x + w, y + h));
    // std::cout << *_selectedArea << std::endl;
    _selectedAreaIntersects = occupies;


    for(auto di: _newSelection) {
        di->selected(false);
    }

    for(auto di: _selectedDrawables) {
        di->selected(true);
    }

    lc::EntityContainer<lc::entity::CADEntity_CSPtr> entitiesInSelection;

    if (occupies) {
        entitiesInSelection = _document->entityContainer().entitiesFullWithinArea(*_selectedArea);
    }
    else {
        entitiesInSelection = _document->entityContainer().entitiesWithinAndCrossingArea(*_selectedArea);
    }
    _newSelection.clear();
    entitiesInSelection.each< const lc::entity::CADEntity >([&](lc::entity::CADEntity_CSPtr entity) {
        auto di = _entityDrawItem[entity];
        auto iter = std::find(_selectedDrawables.begin(), _selectedDrawables.end(), di);
        _newSelection.push_back(di);
        di->selected(!entity || iter == _selectedDrawables.end());
    });
}

LCViewer::LCVDrawItem_SPtr DocumentCanvas::getDrawable(lc::entity::CADEntity_CSPtr entity) {
    return _entityDrawItem[entity];
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
    for(auto drawable: _newSelection) {
        auto iter = std::find(_selectedDrawables.begin(), _selectedDrawables.end(), drawable);
        if(iter != _selectedDrawables.end()) {
            drawable->selected(false);
            _selectedDrawables.erase(iter);
        }
        else {
            drawable->selected(true);
            _selectedDrawables.push_back(drawable);
        }
    };

    _newSelection.clear();
}

void DocumentCanvas::removeSelectionArea() {
    if (_selectedArea != nullptr) {
        delete _selectedArea;
        _selectedArea = nullptr;
    }
}

void DocumentCanvas::removeSelection() {
    for(auto di: _selectedDrawables) {
        di->selected(false);
    };

    _selectedDrawables.clear();
}

Nano::Signal<void(DrawEvent const & event)> & DocumentCanvas::background ()  {
    return _background;
}
Nano::Signal<void(DrawEvent const & event)> & DocumentCanvas::foreground ()  {
    return _foreground;
}

LCVDrawItem_SPtr DocumentCanvas::asDrawable(const lc::entity::CADEntity_CSPtr& entity) {
    // Add a line
    auto line = std::dynamic_pointer_cast<const lc::entity::Line>(entity);

    if (line != nullptr) {
        return std::make_shared<LCVLine>(line);
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

    if (coord != nullptr) {
        return std::make_shared<LCVPoint>(coord);
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

std::vector<LCViewer::LCVDrawItem_SPtr>& DocumentCanvas::selectedDrawables() {
    return _selectedDrawables;
}

lc::EntityContainer<lc::entity::CADEntity_CSPtr> DocumentCanvas::selectedEntities() {
    lc::EntityContainer<lc::entity::CADEntity_CSPtr> entitiesInSelection;
    for(auto di: _selectedDrawables) {
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

    lc::geo::Area selectionArea(lc::geo::Coordinate(x - w, y - w), w * 2, w * 2);
    auto entities = _document->entityContainer().entitiesWithinAndCrossingAreaFast(selectionArea);
    entities.each< const lc::entity::CADEntity >([=](lc::entity::CADEntity_CSPtr entity) {
        _entityDrawItem[entity]->selected(!_entityDrawItem[entity]->selected());
    });
}
