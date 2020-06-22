#include <gtest/gtest.h>
#include "widgets/layers.h"

#include "uitests.h"

#include "dialogs/addlayerdialog.h"

TEST(LayersTest, Creation) {
    QApplication app(argc, argv);
    auto mdiChild = new CadMdiChild;
    mdiChild->newDocument();

    auto layers = new LayersTest(mdiChild);
    auto layer = std::make_shared<const lc::meta::Layer>("Name", lc::meta::MetaLineWidthByValue(1), lc::Color(255,255,255,255));

    /*
     * Creation when document exists
     */

    layers->addLayer(layer);
    EXPECT_EQ(2, layers->nbLayers()) << "Layer was not appended to document";

    /*
     * Creation when document doesn't exist
     */

    layers->setMdiChild(nullptr);
    ASSERT_EQ(0, layers->nbLayers()) << "Unexisting document contains layers";

    layers->addLayer(layer);
    EXPECT_EQ(0, layers->nbLayers()) << "Layer was added to unexisting document";

    delete layers;
    delete mdiChild;
}

TEST(LayersTest, Selection) {
    QApplication app(argc, argv);
    auto mdiChild = new CadMdiChild;
    mdiChild->newDocument();

    auto layers = new LayersTest(mdiChild);
    auto layer = std::make_shared<const lc::meta::Layer>("Name", lc::meta::MetaLineWidthByValue(1), lc::Color(255,255,255,255));

    /*
     * Select layer
     */
    layers->addLayer(layer);
    ASSERT_EQ(2, layers->nbLayers()) << "Layer was not added to document";

    layers->selectLayer(0);
    auto selectedLayer1 = mdiChild->activeLayer();

    layers->selectLayer(1);
    auto selectedLayer2 = mdiChild->activeLayer();

    EXPECT_TRUE((selectedLayer1 == layer) ^ (selectedLayer2 == layer)) << "Wrong layer selected";

    delete layers;
    delete mdiChild;
}

TEST(LayersTest, Edition) {
    QApplication app(argc, argv);
    auto mdiChild = new CadMdiChild;
    mdiChild->newDocument();

    auto layers = new LayersTest(mdiChild);
    auto layer = std::make_shared<const lc::meta::Layer>("Name", lc::meta::MetaLineWidthByValue(1), lc::Color(255,255,255,255));

    layers->addLayer(layer);
    ASSERT_EQ(2, layers->nbLayers()) << "Layer was not added to document";

    /*
     * Trigger locked
     */
    bool locked = mdiChild->activeLayer()->isFrozen();

    layers->cellClick(LayerModel::LOCKED, 0);
    locked = !locked;

    ASSERT_EQ(locked, mdiChild->activeLayer()->isFrozen()) << "Locked value didn't change";

    layers->cellClick(LayerModel::LOCKED, 0);
    locked = !locked;

    EXPECT_EQ(locked, mdiChild->activeLayer()->isFrozen()) << "Locked value didn't change";

    delete layers;
    delete mdiChild;
}

TEST(LayersTest, Remove) {
    QApplication app(argc, argv);
    auto mdiChild = new CadMdiChild;
    mdiChild->newDocument();

    auto layers = new LayersTest(mdiChild);
    auto layer = std::make_shared<const lc::meta::Layer>("Name", lc::meta::MetaLineWidthByValue(1), lc::Color(255,255,255,255));

    layers->addLayer(layer);
    ASSERT_EQ(2, layers->nbLayers()) << "Layer was not added to document";

    layers->selectLayer(0);
    ASSERT_EQ(layer, mdiChild->activeLayer());

    layers->remove();
    EXPECT_EQ(1, layers->nbLayers()) << "Layer was not removed";

    delete layers;
    delete mdiChild;
}

TEST(LayersTest, EditionDialog) {
    QApplication app(argc, argv);
    auto document = createDocument();

    auto width = 0.13;
    auto color = lc::Color(1, 2, 3, 4);
    auto linePattern = std::make_shared<const lc::meta::DxfLinePatternByValue>("LP", "Line Pattern", std::vector<double>{1, -1, 0, -1}, 3);
    auto layer = std::make_shared<const lc::meta::Layer>("Layer", lc::meta::MetaLineWidthByValue(width), color, linePattern, false);

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

TEST(LayersTest, AddAndGetAPI) {
    QApplication app(argc, argv);
    auto mdiChild = new lc::ui::CadMdiChild;
    mdiChild->newDocument();

    auto layers = new lc::ui::widgets::Layers(mdiChild, nullptr);

    auto layer1 = std::make_shared<const lc::meta::Layer>("TestLayer1", lc::meta::MetaLineWidthByValue(1), lc::Color(255, 255, 255, 255));
    layers->addLayer(layer1);
    EXPECT_EQ(layers->layerByName("TestLayer1"), layer1);

    auto layer2 = layers->addLayer("TestLayer2");
    EXPECT_EQ(layers->layerByName("TestLayer2"), layer2);

    auto layer3 = layers->addLayer("TestLayer3", 2.00);
    EXPECT_EQ(layers->layerByName("TestLayer3"), layer3);

    auto layer4 = layers->addLayer("TestLayer4", 0, 255, 0);
    EXPECT_EQ(layers->layerByName("TestLayer4"), layer4);

    auto layer5 = layers->addLayer("TestLayer5", 2.00, 0, 255, 0);
    EXPECT_EQ(layers->layerByName("TestLayer5"), layer5);

    auto layer6 = layers->addLayer("TestLayer6", 2.00, lc::Color(0.0,1.0,0.0));
    EXPECT_EQ(layers->layerByName("TestLayer6"), layer6);
}

TEST(LayersTest, APITest) {
    QApplication app(argc, argv);
    auto mdiChild = new lc::ui::CadMdiChild;
    mdiChild->newDocument();

    auto layers = new lc::ui::widgets::Layers(mdiChild, nullptr);

    auto layer1 = std::make_shared<const lc::meta::Layer>("TestLayer1", lc::meta::MetaLineWidthByValue(1), lc::Color(255, 255, 255, 255));
    layers->addLayer(layer1);
    EXPECT_EQ(layers->layerByName("TestLayer1"), layer1);

    layers->removeLayer("TestLayer1");
    EXPECT_EQ(layers->layerByName("TestLayer1"), nullptr);

    auto layer2 = layers->addLayer("TestLayer2");
    layers->renameLayer("TestLayer2", "RenamedLayer");
    EXPECT_TRUE(layers->layerByName("RenamedLayer") != nullptr);

    auto layer3 = layers->addLayer("TestLayer3");
    layers->removeLayer(layer3);
    EXPECT_TRUE(layers->layerByName("TestLayer3") == nullptr);
}
