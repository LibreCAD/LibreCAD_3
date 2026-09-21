// Menu entries MainWindow connects itself rather than through an operation's
// menu_actions, driven the way a user drives them: by triggering the entry,
// then giving a point, here typed at the command line.

#include <QApplication>
#include <QKeySequence>

#include <gtest/gtest.h>

#include <mainwindow.h>
#include <cad/meta/layer.h>
#include <cad/operations/entitybuilder.h>
#include <cad/operations/layerops.h>
#include <cad/primitive/arc.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/dimangular.h>
#include <cad/primitive/line.h>

#include "uitests.h"

namespace {

std::vector<lc::entity::CADEntity_CSPtr> entitiesOf(lc::ui::MainWindow& window) {
    return window.cadMdiChild()->document()->entityContainer().asVector();
}

void trigger(lc::ui::MainWindow& window, const char* action) {
    window.findMenuItemByObjectName(action)->trigger();
}

void addLine(lc::ui::MainWindow& window, const lc::geo::Coordinate& start, const lc::geo::Coordinate& end,
             const lc::meta::Layer_CSPtr& layer) {
    auto builder = std::make_shared<lc::operation::EntityBuilder>(window.cadMdiChild()->document());
    builder->appendEntity(std::make_shared<lc::entity::Line>(start, end, layer));
    builder->execute();
}

template <typename Kind>
std::shared_ptr<const Kind> lastOf(lc::ui::MainWindow& window) {
    std::shared_ptr<const Kind> last;
    for (const auto& entity : entitiesOf(window)) {
        auto kind = std::dynamic_pointer_cast<const Kind>(entity);
        if (kind != nullptr && (last == nullptr || kind->id() > last->id())) {
            last = kind;
        }
    }
    return last;
}

bool near(const lc::geo::Coordinate& a, const lc::geo::Coordinate& b) {
    return a.distanceTo(b) < 1e-9;
}

/// Whether the drawing has a line from start to end on a layer named layerName.
bool hasLine(lc::ui::MainWindow& window, const lc::geo::Coordinate& start, const lc::geo::Coordinate& end,
             const std::string& layerName) {
    for (const auto& entity : entitiesOf(window)) {
        auto line = std::dynamic_pointer_cast<const lc::entity::Line>(entity);
        if (line != nullptr && line->start() == start && line->end() == end
            && line->layer()->name() == layerName) {
            return true;
        }
    }
    return false;
}

template <typename Kind>
size_t countOf(lc::ui::MainWindow& window) {
    size_t count = 0;
    for (const auto& entity : entitiesOf(window)) {
        if (std::dynamic_pointer_cast<const Kind>(entity) != nullptr) {
            count++;
        }
    }
    return count;
}

}  // namespace

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MenuEntriesTest, CutCopyAndPasteHaveTheirShortcuts) {
    QApplication app(argc, argv);
    auto* window = new lc::ui::MainWindow();

    EXPECT_EQ(window->findMenuItemByObjectName("actionCut")->shortcut(), QKeySequence(QKeySequence::Cut));
    EXPECT_EQ(window->findMenuItemByObjectName("actionCopy")->shortcut(), QKeySequence(QKeySequence::Copy));
    EXPECT_EQ(window->findMenuItemByObjectName("actionPaste")->shortcut(), QKeySequence(QKeySequence::Paste));
}

// Paste puts the lower left corner of what was copied where the user clicks,
// and takes one Undo to take back.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MenuEntriesTest, CopyThenPastePutsTheCopiesWhereTheUserClicks) {
    QApplication app(argc, argv);
    auto* window = new lc::ui::MainWindow();
    const auto layer = window->cadMdiChild()->activeLayer();

    addLine(*window, {10, 20}, {30, 25}, layer);
    addLine(*window, {15, 40}, {12, 22}, layer);
    const size_t before = entitiesOf(*window).size();

    trigger(*window, "actionSelect_All");
    trigger(*window, "actionCopy");
    trigger(*window, "actionPaste");
    window->triggerCoordinateEntered(lc::geo::Coordinate(100, 200));

    ASSERT_EQ(entitiesOf(*window).size(), before + 2);
    EXPECT_TRUE(hasLine(*window, {100, 200}, {120, 205}, layer->name()));
    EXPECT_TRUE(hasLine(*window, {105, 220}, {102, 202}, layer->name()));
    EXPECT_TRUE(hasLine(*window, {10, 20}, {30, 25}, layer->name())) << "the original stays";

    trigger(*window, "actionUndo");
    EXPECT_EQ(entitiesOf(*window).size(), before);
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MenuEntriesTest, CutRemovesTheSelectionInOneUndoStepAndPastesBack) {
    QApplication app(argc, argv);
    auto* window = new lc::ui::MainWindow();
    const auto layer = window->cadMdiChild()->activeLayer();

    const size_t before = entitiesOf(*window).size();
    addLine(*window, {0, 0}, {10, 0}, layer);
    addLine(*window, {0, 5}, {10, 5}, layer);

    trigger(*window, "actionSelect_All");
    trigger(*window, "actionCut");
    EXPECT_EQ(entitiesOf(*window).size(), before);

    trigger(*window, "actionUndo");
    EXPECT_EQ(entitiesOf(*window).size(), before + 2);
    trigger(*window, "actionRedo");
    EXPECT_EQ(entitiesOf(*window).size(), before);

    trigger(*window, "actionPaste");
    window->triggerCoordinateEntered(lc::geo::Coordinate(50, 50));
    EXPECT_TRUE(hasLine(*window, {50, 50}, {60, 50}, layer->name()));
    EXPECT_TRUE(hasLine(*window, {50, 55}, {60, 55}, layer->name()));
}

// A layer the other drawing does not have comes along with the entities on
// it, in the same undo step, rather than the entities landing on whatever
// layer happens to be active there.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MenuEntriesTest, PastingIntoAnotherDrawingBringsTheLayerAlong) {
    QApplication app(argc, argv);
    auto* from = new lc::ui::MainWindow();
    auto* to = new lc::ui::MainWindow();

    auto walls = std::make_shared<const lc::meta::Layer>("Walls");
    std::make_shared<lc::operation::AddLayer>(from->cadMdiChild()->document(), walls)->execute();
    addLine(*from, {1, 1}, {4, 1}, walls);

    trigger(*from, "actionSelect_All");
    trigger(*from, "actionCopy");

    ASSERT_EQ(to->cadMdiChild()->document()->layerByName("Walls"), nullptr);
    const size_t before = entitiesOf(*to).size();

    trigger(*to, "actionPaste");
    to->triggerCoordinateEntered(lc::geo::Coordinate(11, 11));

    EXPECT_NE(to->cadMdiChild()->document()->layerByName("Walls"), nullptr);
    EXPECT_TRUE(hasLine(*to, {11, 11}, {14, 11}, "Walls"));

    trigger(*to, "actionUndo");
    EXPECT_EQ(entitiesOf(*to).size(), before);
    EXPECT_EQ(to->cadMdiChild()->document()->layerByName("Walls"), nullptr);
}

// Create > Add Random Lines, Circles and Arc add a thousand of their kind
// each, as one undo step.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MenuEntriesTest, AddRandomEntriesAddAThousandOfTheirKindInOneUndoStep) {
    QApplication app(argc, argv);
    auto* window = new lc::ui::MainWindow();
    const size_t before = entitiesOf(*window).size();

    trigger(*window, "actionAdd_Random_Lines");
    EXPECT_EQ(countOf<lc::entity::Line>(*window), 1000u);
    trigger(*window, "actionAdd_Random_Circles");
    EXPECT_EQ(countOf<lc::entity::Circle>(*window), 1000u);
    trigger(*window, "actionAdd_Random_Arc");
    EXPECT_EQ(countOf<lc::entity::Arc>(*window), 1000u);
    EXPECT_EQ(entitiesOf(*window).size(), before + 3000);

    trigger(*window, "actionUndo");
    EXPECT_EQ(countOf<lc::entity::Arc>(*window), 0u);
    EXPECT_EQ(entitiesOf(*window).size(), before + 2000);
}

// Arc > Continue starts where the last line or arc ends, going the way it
// went, and ends where the user says.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MenuEntriesTest, ContinueDrawsAnArcOnFromTheLastLineOrArc) {
    QApplication app(argc, argv);
    auto* window = new lc::ui::MainWindow();
    addLine(*window, {0, 0}, {10, 0}, window->cadMdiChild()->activeLayer());

    trigger(*window, "actionContinue");
    window->triggerCoordinateEntered(lc::geo::Coordinate(20, 10));

    auto arc = lastOf<lc::entity::Arc>(*window);
    ASSERT_NE(arc, nullptr);
    EXPECT_TRUE(near(arc->center(), lc::geo::Coordinate(10, 10)));
    EXPECT_NEAR(arc->radius(), 10, 1e-9);
    EXPECT_TRUE(arc->CCW());
    EXPECT_TRUE(near(arc->startP(), lc::geo::Coordinate(10, 0)));
    EXPECT_TRUE(near(arc->endP(), lc::geo::Coordinate(20, 10)));

    // Heading up from (20, 10) now: the next quarter of the same circle.
    trigger(*window, "actionContinue");
    window->triggerCoordinateEntered(lc::geo::Coordinate(10, 20));

    auto next = lastOf<lc::entity::Arc>(*window);
    ASSERT_NE(next, arc);
    EXPECT_TRUE(near(next->center(), lc::geo::Coordinate(10, 10)));
    EXPECT_NEAR(next->radius(), 10, 1e-9);
    EXPECT_TRUE(near(next->endP(), lc::geo::Coordinate(10, 20)));
}

// Dimension > ANG2LN: click on two lines, then where the arc goes.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MenuEntriesTest, AngularDimensionFromTwoLinesPickedByClicking) {
    QApplication app(argc, argv);
    auto* window = new lc::ui::MainWindow();
    const auto layer = window->cadMdiChild()->activeLayer();
    addLine(*window, {2, 2}, {12, 2}, layer);
    addLine(*window, {2, 2}, {2, 12}, layer);
    addLine(*window, {40, 40}, {50, 45}, layer);

    trigger(*window, "actionANG2LN");
    window->triggerCoordinateEntered(lc::geo::Coordinate(7, 2.5));
    window->triggerCoordinateEntered(lc::geo::Coordinate(2.5, 7));
    window->triggerCoordinateEntered(lc::geo::Coordinate(5, 5));

    auto dimension = lastOf<lc::entity::DimAngular>(*window);
    ASSERT_NE(dimension, nullptr);
    EXPECT_EQ(dimension->definitionPoint(), lc::geo::Coordinate(2, 2));
    EXPECT_EQ(dimension->defLine11(), lc::geo::Coordinate(2, 2));
    EXPECT_EQ(dimension->defLine12(), lc::geo::Coordinate(12, 2));
    EXPECT_EQ(dimension->defLine21(), lc::geo::Coordinate(2, 2));
    EXPECT_EQ(dimension->defLine22(), lc::geo::Coordinate(2, 12));
    EXPECT_EQ(dimension->middleOfText(), lc::geo::Coordinate(5, 5));
}

// Two lines already selected are the two lines: only the arc is asked for.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MenuEntriesTest, AngularDimensionFromTwoSelectedLines) {
    QApplication app(argc, argv);
    auto* window = new lc::ui::MainWindow();
    const auto layer = window->cadMdiChild()->activeLayer();
    addLine(*window, {0, 0}, {10, 0}, layer);
    addLine(*window, {10, 10}, {20, 20}, layer);

    trigger(*window, "actionSelect_All");
    trigger(*window, "actionANG2LN");
    window->triggerCoordinateEntered(lc::geo::Coordinate(8, 1));

    auto dimension = lastOf<lc::entity::DimAngular>(*window);
    ASSERT_NE(dimension, nullptr);
    EXPECT_EQ(dimension->definitionPoint(), lc::geo::Coordinate(0, 0));
    EXPECT_EQ(dimension->middleOfText(), lc::geo::Coordinate(8, 1));
}
