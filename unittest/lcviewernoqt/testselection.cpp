#include <gtest/gtest.h>
#include "documentcanvas.h"
#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <cad/storage/undomanagerimpl.h>

#include <cad/operations/entitybuilder.h>
#include <cad/operations/documentoperation.h>
#include <cad/meta/layer.h>
#include <cad/operations/layerops.h>
#include <cad/primitive/line.h>
#include <cad/primitive/point.h>
#include "drawitems/lcvdrawitem.h"
#include "drawables/lccursor.h"
#include "drawables/tempentities.h"
#include "events/snappointevent.h"
#include "managers/dragmanager.h"
#include "managers/snapmanagerimpl.h"

TEST(SelectionTest, ElevatedPointUsesPlanarSelectionAndSnap) {
    auto document = std::make_shared<lc::storage::DocumentImpl>(
        std::make_shared<lc::storage::StorageManagerImpl>());
    auto point = std::make_shared<lc::entity::Point>(lc::geo::Coordinate(3, 4, 100), document->layerByName("0"));
    document->insertEntity(point);
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(document);
    canvas->selectPoint(3, 4);
    EXPECT_EQ(canvas->selectedDrawables().size(), 1u);
    if (!canvas->selectedDrawables().empty())
        EXPECT_EQ(canvas->selectedDrawables().front()->entity(), point);

    lc::viewer::manager::SnapManagerImpl snap(canvas, nullptr, 10);
    snap.setMiddleSnappable(true);
    struct Capture : Nano::Observer {
        lc::geo::Coordinate value;
        void receive(const lc::viewer::event::SnapPointEvent& event) { value = event.snapPoint(); }
    } capture;
    snap.snapPointEvents().connect<Capture, &Capture::receive>(capture);
    snap.setDeviceLocation(3, 4);
    EXPECT_DOUBLE_EQ(capture.value.x(), 3);
    EXPECT_DOUBLE_EQ(capture.value.y(), 4);
    EXPECT_DOUBLE_EQ(capture.value.z(), 100);
    snap.setMiddleSnappable(false); snap.setDeviceLocation(3, 4);
    EXPECT_DOUBLE_EQ(capture.value.z(), 0);
    // The entity's 3D API still uses full 3D distance.
    EXPECT_TRUE(point->snapPoints(lc::geo::Coordinate(3, 4, 0), lc::SimpleSnapConstrain(), 5, 1).empty());
}

TEST(SelectionTest, SnapChoosesNearestPointInCanvasPlane) {
    auto document = std::make_shared<lc::storage::DocumentImpl>(
        std::make_shared<lc::storage::StorageManagerImpl>());
    auto layer = document->layerByName("0");
    document->insertEntity(std::make_shared<lc::entity::Point>(lc::geo::Coordinate(3, 4, 100), layer));
    document->insertEntity(std::make_shared<lc::entity::Point>(lc::geo::Coordinate(4, 4, 0), layer));
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(document);
    lc::viewer::manager::SnapManagerImpl snap(canvas, nullptr, 10);
    snap.setMiddleSnappable(true);
    struct Capture : Nano::Observer {
        lc::geo::Coordinate value;
        void receive(const lc::viewer::event::SnapPointEvent& event) { value = event.snapPoint(); }
    } capture;
    snap.snapPointEvents().connect<Capture, &Capture::receive>(capture);
    snap.setDeviceLocation(3, 4);
    EXPECT_DOUBLE_EQ(capture.value.x(), 3);
    EXPECT_DOUBLE_EQ(capture.value.z(), 100);
    snap.setDeviceLocation(4, 4);
    EXPECT_DOUBLE_EQ(capture.value.x(), 4);
    EXPECT_DOUBLE_EQ(capture.value.z(), 0);
}

TEST(SelectionTest, DragElevatedPointPreservesElevationAcrossMoves) {
    auto document = std::make_shared<lc::storage::DocumentImpl>(
        std::make_shared<lc::storage::StorageManagerImpl>());
    auto history = std::make_shared<lc::storage::UndoManagerImpl>(10);
    document->commitProcessEvent().connect<lc::storage::UndoManagerImpl,
        &lc::storage::UndoManagerImpl::on_CommitProcessEvent>(history.get());
    auto point = std::make_shared<lc::entity::Point>(lc::geo::Coordinate(3, 4, 100), document->layerByName("0"));
    document->insertEntity(point);
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(document);
    canvas->selectEntity(point);
    auto cursor = std::make_shared<lc::viewer::drawable::Cursor>(10, canvas, lc::Color(), lc::Color());
    auto temporary = std::make_shared<lc::viewer::drawable::TempEntities>(canvas);
    lc::viewer::manager::DragManager drag(canvas, cursor, temporary, 10);
    cursor->onSnapPointEvent(lc::viewer::event::SnapPointEvent(lc::geo::Coordinate(3, 4)));
    drag.onMouseMove(); drag.onMousePress(); ASSERT_TRUE(drag.entityDragged());
    cursor->onSnapPointEvent(lc::viewer::event::SnapPointEvent(lc::geo::Coordinate(8, 9)));
    drag.onMouseMove();
    cursor->onSnapPointEvent(lc::viewer::event::SnapPointEvent(lc::geo::Coordinate(9, 10)));
    drag.onMouseMove(); drag.onMouseRelease();
    const auto moved = std::dynamic_pointer_cast<const lc::entity::Point>(document->entityByID(point->id()));
    ASSERT_TRUE(moved);
    EXPECT_DOUBLE_EQ(moved->x(), 9); EXPECT_DOUBLE_EQ(moved->y(), 10); EXPECT_DOUBLE_EQ(moved->z(), 100);
    ASSERT_TRUE(history->canUndo()); history->undo();
    const auto restored = std::dynamic_pointer_cast<const lc::entity::Point>(document->entityByID(point->id()));
    ASSERT_TRUE(restored); EXPECT_DOUBLE_EQ(restored->x(), 3); EXPECT_DOUBLE_EQ(restored->z(), 100);
    ASSERT_TRUE(history->canRedo()); history->redo();
    EXPECT_EQ(document->entityByID(point->id()), moved);
}

TEST(SelectionTest, DragCoincidentPointsPreservesEachElevation) {
    auto document = std::make_shared<lc::storage::DocumentImpl>(
        std::make_shared<lc::storage::StorageManagerImpl>());
    auto history = std::make_shared<lc::storage::UndoManagerImpl>(10);
    document->commitProcessEvent().connect<lc::storage::UndoManagerImpl,
        &lc::storage::UndoManagerImpl::on_CommitProcessEvent>(history.get());
    auto layer = document->layerByName("0");
    auto first = std::make_shared<lc::entity::Point>(lc::geo::Coordinate(3, 4, 20), layer);
    auto second = std::make_shared<lc::entity::Point>(lc::geo::Coordinate(3, 4, 100), layer);
    document->insertEntity(first);
    document->insertEntity(second);
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(document);
    canvas->selectEntity(first);
    canvas->selectEntity(second);
    auto cursor = std::make_shared<lc::viewer::drawable::Cursor>(10, canvas, lc::Color(), lc::Color());
    auto temporary = std::make_shared<lc::viewer::drawable::TempEntities>(canvas);
    lc::viewer::manager::DragManager drag(canvas, cursor, temporary, 10);
    cursor->onSnapPointEvent(lc::viewer::event::SnapPointEvent(lc::geo::Coordinate(3, 4)));
    drag.onMouseMove(); drag.onMousePress(); ASSERT_TRUE(drag.entityDragged());
    cursor->onSnapPointEvent(lc::viewer::event::SnapPointEvent(lc::geo::Coordinate(8, 9)));
    drag.onMouseMove();
    cursor->onSnapPointEvent(lc::viewer::event::SnapPointEvent(lc::geo::Coordinate(9, 10)));
    drag.onMouseMove(); drag.onMouseRelease();
    const auto a = std::dynamic_pointer_cast<const lc::entity::Point>(document->entityByID(first->id()));
    const auto b = std::dynamic_pointer_cast<const lc::entity::Point>(document->entityByID(second->id()));
    ASSERT_TRUE(a); ASSERT_TRUE(b);
    EXPECT_DOUBLE_EQ(a->x(), 9); EXPECT_DOUBLE_EQ(a->y(), 10); EXPECT_DOUBLE_EQ(a->z(), 20);
    EXPECT_DOUBLE_EQ(b->x(), 9); EXPECT_DOUBLE_EQ(b->y(), 10); EXPECT_DOUBLE_EQ(b->z(), 100);
    ASSERT_TRUE(history->canUndo()); history->undo();
    const auto restoredA = std::dynamic_pointer_cast<const lc::entity::Point>(document->entityByID(first->id()));
    const auto restoredB = std::dynamic_pointer_cast<const lc::entity::Point>(document->entityByID(second->id()));
    ASSERT_TRUE(restoredA); ASSERT_TRUE(restoredB);
    EXPECT_DOUBLE_EQ(restoredA->x(), 3); EXPECT_DOUBLE_EQ(restoredA->z(), 20);
    EXPECT_DOUBLE_EQ(restoredB->x(), 3); EXPECT_DOUBLE_EQ(restoredB->z(), 100);
    ASSERT_TRUE(history->canRedo()); history->redo();
    EXPECT_EQ(document->entityByID(first->id()), a);
    EXPECT_EQ(document->entityByID(second->id()), b);
}

TEST(SelectionTest, DragOverlappingLineGripsUpdatesEntityOnce) {
    auto document = std::make_shared<lc::storage::DocumentImpl>(
        std::make_shared<lc::storage::StorageManagerImpl>());
    auto history = std::make_shared<lc::storage::UndoManagerImpl>(10);
    document->commitProcessEvent().connect<lc::storage::UndoManagerImpl,
        &lc::storage::UndoManagerImpl::on_CommitProcessEvent>(history.get());
    auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(3, 4, 20),
                                                  lc::geo::Coordinate(3, 4, 100), document->layerByName("0"));
    document->insertEntity(line);
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(document);
    canvas->selectEntity(line);
    struct Capture : Nano::Observer {
        int added = 0;
        int removed = 0;
        void onAdd(const lc::event::AddEntityEvent&) { ++added; }
        void onRemove(const lc::event::RemoveEntityEvent&) { ++removed; }
    } capture;
    document->addEntityEvent().connect<Capture, &Capture::onAdd>(capture);
    document->removeEntityEvent().connect<Capture, &Capture::onRemove>(capture);
    auto cursor = std::make_shared<lc::viewer::drawable::Cursor>(10, canvas, lc::Color(), lc::Color());
    auto temporary = std::make_shared<lc::viewer::drawable::TempEntities>(canvas);
    lc::viewer::manager::DragManager drag(canvas, cursor, temporary, 10);
    cursor->onSnapPointEvent(lc::viewer::event::SnapPointEvent(lc::geo::Coordinate(3, 4)));
    drag.onMouseMove(); drag.onMousePress(); ASSERT_TRUE(drag.entityDragged());
    cursor->onSnapPointEvent(lc::viewer::event::SnapPointEvent(lc::geo::Coordinate(9, 10)));
    drag.onMouseMove(); drag.onMouseRelease();
    const auto moved = std::dynamic_pointer_cast<const lc::entity::Line>(document->entityByID(line->id()));
    ASSERT_TRUE(moved);
    EXPECT_DOUBLE_EQ(moved->start().x(), 9); EXPECT_DOUBLE_EQ(moved->start().y(), 10);
    EXPECT_DOUBLE_EQ(moved->start().z(), 20);
    EXPECT_DOUBLE_EQ(moved->end().x(), 9); EXPECT_DOUBLE_EQ(moved->end().y(), 10);
    EXPECT_DOUBLE_EQ(moved->end().z(), 100);
    EXPECT_EQ(capture.removed, 1); EXPECT_EQ(capture.added, 1);
    ASSERT_TRUE(history->canUndo()); history->undo();
    EXPECT_EQ(document->entityByID(line->id()), line);
    ASSERT_TRUE(history->canRedo()); history->redo();
    EXPECT_EQ(document->entityByID(line->id()), moved);
    document->addEntityEvent().disconnect<Capture, &Capture::onAdd>(capture);
    document->removeEntityEvent().disconnect<Capture, &Capture::onRemove>(capture);
}

TEST(SelectionTest, ElevatedPointOnFrozenLayerIsNotSnapped) {
    auto document = std::make_shared<lc::storage::DocumentImpl>(
        std::make_shared<lc::storage::StorageManagerImpl>());
    auto layer = std::make_shared<lc::meta::Layer>("FROZEN", lc::meta::MetaLineWidthByValue(1),
                                                   lc::Color(), nullptr, true);
    document->addDocumentMetaType(layer);
    document->insertEntity(std::make_shared<lc::entity::Point>(lc::geo::Coordinate(3, 4, 100), layer));
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(document);
    lc::viewer::manager::SnapManagerImpl snap(canvas, nullptr, 10); snap.setMiddleSnappable(true);
    struct Capture : Nano::Observer {
        lc::geo::Coordinate value;
        void receive(const lc::viewer::event::SnapPointEvent& event) { value = event.snapPoint(); }
    } capture;
    snap.snapPointEvents().connect<Capture, &Capture::receive>(capture);
    snap.setDeviceLocation(3, 4);
    EXPECT_DOUBLE_EQ(capture.value.z(), 0);
}

TEST(SelectionTest, ElevatedPointOnOffLayerIsNotSnapped) {
    auto document = std::make_shared<lc::storage::DocumentImpl>(
        std::make_shared<lc::storage::StorageManagerImpl>());
    auto layer = std::make_shared<lc::meta::Layer>("OFF", lc::meta::MetaLineWidthByValue(1),
                                                   lc::Color(), nullptr, false, true);
    document->addDocumentMetaType(layer);
    document->insertEntity(std::make_shared<lc::entity::Point>(lc::geo::Coordinate(3, 4, 100), layer));
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(document);
    lc::viewer::manager::SnapManagerImpl snap(canvas, nullptr, 10); snap.setMiddleSnappable(true);
    struct Capture : Nano::Observer {
        lc::geo::Coordinate value;
        void receive(const lc::viewer::event::SnapPointEvent& event) { value = event.snapPoint(); }
    } capture;
    snap.snapPointEvents().connect<Capture, &Capture::receive>(capture);
    snap.setDeviceLocation(3, 4);
    EXPECT_DOUBLE_EQ(capture.value.z(), 0);
}

TEST(SelectionTest, MutatorsNotifyObservers) {
    auto document = std::make_shared<lc::storage::DocumentImpl>(
        std::make_shared<lc::storage::StorageManagerImpl>());
    auto canvas = std::make_shared<lc::viewer::DocumentCanvas>(document);
    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(
        lc::geo::Coordinate(0, 0), lc::geo::Coordinate(0, 10), document->layerByName("0")));
    builder->execute();
    struct Counter : Nano::Observer {
        int count = 0;
        void changed() { ++count; }
    } counter;
    canvas->selectionChanged().connect<Counter, &Counter::changed>(counter);
    canvas->selectAll();
    EXPECT_EQ(canvas->selectedEntities().asVector().size(), 1u);
    EXPECT_EQ(counter.count, 1);
    canvas->removeSelection();
    EXPECT_TRUE(canvas->selectedEntities().asVector().empty());
    EXPECT_EQ(counter.count, 2);
    canvas->inverseSelection();
    EXPECT_EQ(canvas->selectedEntities().asVector().size(), 1u);
    EXPECT_EQ(counter.count, 3);
}

TEST(SelectionTest, NormalSelection) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    auto docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document);

    auto layer = std::make_shared<lc::meta::Layer>();
    std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
    al->execute();

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
    builder->execute();

    docCanvas->makeSelection(0, 0, 5, 10, true);
    docCanvas->closeSelection();

    EXPECT_EQ(1, docCanvas->selectedDrawables().size());

    unsigned int i = 0;

    docCanvas->entityContainer().each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr di) {
        if(docCanvas->getDrawable(di)->selected()) {
            i++;
        }
    });

    EXPECT_TRUE(i == docCanvas->selectedDrawables().size());
}

TEST(SelectionTest, IntersectionSelection) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    auto docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document);

    auto layer = std::make_shared<lc::meta::Layer>();
    std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
    al->execute();

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
    builder->execute();

    docCanvas->makeSelection(0, 0, 5, 5, false);
    docCanvas->closeSelection();

    EXPECT_EQ(1, docCanvas->selectedDrawables().size());

    unsigned int i = 0;

    docCanvas->entityContainer().each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr di) {
        if(docCanvas->getDrawable(di)->selected()) {
            i++;
        }
    });

    EXPECT_TRUE(i == docCanvas->selectedDrawables().size());
}

TEST(SelectionTest, AddToSelection) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    auto docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document);

    auto layer = std::make_shared<lc::meta::Layer>();
    std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
    al->execute();

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
    builder->execute();

    docCanvas->makeSelection(0, 0, 1, 1, false);
    docCanvas->closeSelection();

    docCanvas->makeSelection(9, 0, 1, 1, false);
    docCanvas->closeSelection();

    EXPECT_EQ(2, docCanvas->selectedDrawables().size());

    unsigned int i = 0;

    docCanvas->entityContainer().each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr di) {
        if(docCanvas->getDrawable(di)->selected()) {
            i++;
        }
    });

    EXPECT_TRUE(i == docCanvas->selectedDrawables().size());
}

TEST(SelectionTest, Reselect) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    auto docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document);

    auto layer = std::make_shared<lc::meta::Layer>();
    std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
    al->execute();

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
    builder->execute();

    docCanvas->makeSelection(0, 0, 10, 1, false);
    docCanvas->closeSelection();

    docCanvas->makeSelection(9, 0, 1, 1, false);
    docCanvas->closeSelection();

    EXPECT_EQ(1, docCanvas->selectedDrawables().size());

    unsigned int i = 0;

    docCanvas->entityContainer().each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr di) {
        if(docCanvas->getDrawable(di)->selected()) {
            i++;
        }
    });

    EXPECT_TRUE(i == docCanvas->selectedDrawables().size());
}

TEST(SelectionTest, ClearSelection) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    auto docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document);

    auto layer = std::make_shared<lc::meta::Layer>();
    std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
    al->execute();

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
    builder->execute();

    docCanvas->makeSelection(0, 0, 1, 1, false);
    docCanvas->closeSelection();

    docCanvas->makeSelection(9, 0, 1, 1, false);
    docCanvas->closeSelection();

    EXPECT_EQ(2, docCanvas->selectedDrawables().size());

    unsigned int i = 0;

    docCanvas->entityContainer().each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr di) {
        if(docCanvas->getDrawable(di)->selected()) {
            i++;
        }
    });

    EXPECT_TRUE(i == docCanvas->selectedDrawables().size());
}

TEST(SelectionTest, Deselect) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    auto docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document);

    auto layer = std::make_shared<lc::meta::Layer>();
    std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
    al->execute();

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
    builder->execute();

    docCanvas->makeSelection(0, 0, 1, 1, false);
    docCanvas->makeSelection(-1, -1, 0, 0, false);
    docCanvas->closeSelection();

    EXPECT_EQ(0, docCanvas->selectedDrawables().size());

    unsigned int i = 0;

    docCanvas->entityContainer().each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr di) {
        if(docCanvas->getDrawable(di)->selected()) {
            i++;
        }
    });

    EXPECT_TRUE(i == docCanvas->selectedDrawables().size());
}

TEST(SelectionTest, DeselectAddTo) {
    auto storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    auto document = std::make_shared<lc::storage::DocumentImpl>(storageManager);
    auto docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document);

    auto layer = std::make_shared<lc::meta::Layer>();
    std::shared_ptr<lc::operation::AddLayer> al = std::make_shared<lc::operation::AddLayer>(document, layer);
    al->execute();

    auto builder = std::make_shared<lc::operation::EntityBuilder>(document);
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0,0,0), lc::geo::Coordinate(0, 10, 0), layer));
    builder->appendEntity(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(10,0,0), lc::geo::Coordinate(10, 10, 0), layer));
    builder->execute();

    docCanvas->makeSelection(0, 0, 1, 1, false);
    docCanvas->closeSelection();

    docCanvas->makeSelection(9, 0, 1, 1, false);
    docCanvas->makeSelection(11, 0, 0, 0, false);
    docCanvas->closeSelection();

    EXPECT_EQ(1, docCanvas->selectedDrawables().size());

    unsigned int i = 0;

    docCanvas->entityContainer().each<const lc::entity::CADEntity>([&](lc::entity::CADEntity_CSPtr di) {
        if(docCanvas->getDrawable(di)->selected()) {
            i++;
        }
    });

    EXPECT_TRUE(i == docCanvas->selectedDrawables().size());
}
