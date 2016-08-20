#include <gtest/gtest.h>
#include "uitests.h"
#include "widgets/layers.h"

#include "dialogs/addlayerdialog.h"
#include <cad/meta/metalinewidth.h>

TEST(LayersTest, Creation) {
    QApplication app(argc, argv);
    auto document = createDocument();

    auto layers = new LayersTest(document);
    auto layer = std::make_shared<const lc::Layer>("Name", lc::MetaLineWidthByValue(1), lc::Color(255,255,255,255));

    /*
     * Creation when document exists
     */
    ASSERT_EQ(0, layers->nbLayers()) << "Document contains layers after creation";

    layers->addLayer(layer);
    EXPECT_EQ(1, layers->nbLayers()) << "Layer was not appended to document";

    /*
     * Creation when document doesn't exist
     */

    layers->setDocument(nullptr);
    ASSERT_EQ(0, layers->nbLayers()) << "Unexisting document contains layers";

    layers->addLayer(layer);
    EXPECT_EQ(0, layers->nbLayers()) << "Layer was added to unexisting document";

    delete layers;
}

TEST(LayersTest, Selection) {
    QApplication app(argc, argv);
    auto document = createDocument();

    auto layers = new LayersTest(document);
    auto layer = std::make_shared<const lc::Layer>("Name", lc::MetaLineWidthByValue(1), lc::Color(255,255,255,255));

    /*
     * Selection when no layers are present in the document
     */
    ASSERT_EQ(0, layers->nbLayers()) << "Document contains layers after creation";

    EXPECT_NE(nullptr, layers->activeLayer()) << "Empty layer should be created";
    EXPECT_EQ(1, layers->nbLayers()) << "Empty layer should be added to document";

    /*
     * Select layer
     */
    layers->addLayer(layer);
    ASSERT_EQ(2, layers->nbLayers()) << "Layer was not added to document";

    layers->selectLayer(0);
    auto selectedLayer1 = layers->activeLayer();

    layers->selectLayer(1);
    auto selectedLayer2 = layers->activeLayer();

    EXPECT_TRUE((selectedLayer1 == layer) ^ (selectedLayer2 == layer)) << "Wrong layer selected";

    delete layers;
}

TEST(LayersTest, Edition) {
    QApplication app(argc, argv);
    auto document = createDocument();

    auto layers = new LayersTest(document);
    auto layer = std::make_shared<const lc::Layer>("Name", lc::MetaLineWidthByValue(1), lc::Color(255,255,255,255));

    layers->addLayer(layer);
    ASSERT_EQ(1, layers->nbLayers()) << "Layer was not added to document";

    /*
     * Trigger locked
     */
    bool locked = layers->activeLayer()->isFrozen();

    layers->cellClick(LayerModel::LOCKED, 0);
    locked = !locked;

    ASSERT_EQ(locked, layers->activeLayer()->isFrozen()) << "Locked value didn't change";

    layers->cellClick(LayerModel::LOCKED, 0);
    locked = !locked;

    EXPECT_EQ(locked, layers->activeLayer()->isFrozen()) << "Locked value didn't change";

    delete layers;
}

TEST(LayersTest, Remove) {
    QApplication app(argc, argv);
    auto document = createDocument();

    auto layers = new LayersTest(document);
    auto layer = std::make_shared<const lc::Layer>("Name", lc::MetaLineWidthByValue(1), lc::Color(255,255,255,255));

    layers->addLayer(layer);
    ASSERT_EQ(1, layers->nbLayers()) << "Layer was not added to document";

    layers->selectLayer(0);
    ASSERT_EQ(layer, layers->activeLayer());

    layers->remove();
    EXPECT_EQ(0, layers->nbLayers()) << "Layer was not removed";

    delete layers;
}

TEST(LayersTest, EditionDialog) {
    QApplication app(argc, argv);
    auto document = createDocument();

    auto width = 0.13;
    auto color = lc::Color(1, 2, 3, 4);
    auto linePattern = std::make_shared<const lc::DxfLinePattern>("LP", "Line Pattern", std::vector<double>{1, -1, 0, -1}, 3);
    auto layer = std::make_shared<const lc::Layer>("Layer", lc::MetaLineWidthByValue(width), color, linePattern, false);

    auto operation = std::make_shared<lc::operation::AddLinePattern>(document, linePattern);
    operation->execute();

    AddLayerDialogTest layerDialog(layer, document);

    auto dialogColor = layerDialog.color();

    EXPECT_EQ(layer->name(), layerDialog.layerName());
    EXPECT_EQ(linePattern->name(), layerDialog.linePatternName());
    EXPECT_EQ(width, layerDialog.lineWidthValue());
    EXPECT_TRUE(
            color.red() == dialogColor.red() &&
            color.green() == dialogColor.green() &&
            color.blue() == dialogColor.blue()
    );
}