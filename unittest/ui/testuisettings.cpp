#include <gtest/gtest.h>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QGridLayout>
#include <QPushButton>
#include <QTemporaryDir>
#include <managers/uisettings.h>
#include <widgets/customizeToolbar/customizetoolbar.h>
#include <widgets/toolbar.h>

#include "uitests.h"

namespace {
class TestUiSettings : public lc::ui::UiSettings {
public:
    explicit TestUiSettings(const QString& directory) {
        _filePaths["settings_load"] = (directory + QDir::separator()).toStdString();
    }
};

class UiSettingsTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        (void)ensureUiApplication();
    }
    void SetUp() override { ASSERT_TRUE(directory.isValid()); }

    void writeSettingsFile(const QByteArray& bytes) {
        QFile file(QDir(directory.path()).filePath("ui_settings.json"));
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        ASSERT_EQ(file.write(bytes), bytes.size());
    }

    rapidjson::Document readSettingsFile() {
        QFile file(QDir(directory.path()).filePath("ui_settings.json"));
        if (!file.open(QIODevice::ReadOnly)) {
            ADD_FAILURE() << "Settings were not written";
            return rapidjson::Document();
        }
        const QByteArray bytes = file.readAll();
        rapidjson::Document document;
        document.Parse(bytes.constData());
        EXPECT_FALSE(document.HasParseError());
        return document;
    }

    rapidjson::Document generated() {
        rapidjson::Document document(rapidjson::kObjectType);
        customize.generateData(document);
        return document;
    }

    QTemporaryDir directory;
    TestUiSettings settings{directory.path()};
    lc::ui::widgets::Toolbar toolbar{nullptr};
    lc::ui::widgets::CustomizeToolbar customize{&toolbar};
};
}

TEST_F(UiSettingsTest, DefaultButtonUsesSafeDefaultsWhenUserFileIsMissing) {
    auto* button = customize.findChild<QPushButton*>("defaultButton");
    ASSERT_NE(button, nullptr);
    int requests = 0;
    QObject::connect(&customize, &lc::ui::widgets::CustomizeToolbar::defaultSettingsLoad,
                     [&] { ++requests; settings.readSettings(&customize, true); });
    button->click();
    EXPECT_EQ(requests, 1);
    auto document = generated();
    ASSERT_EQ(document["toolbar"]["tabs"].Size(), 1u);
    EXPECT_EQ(document["toolbar"]["tabs"][0]["groups"].Size(), 4u);
}

TEST_F(UiSettingsTest, MalformedToolbarDocumentsNeverAbort) {
    for (const char* json : {
            "null", "[]", "42", "\"text\"", "{}", "{\"toolbar\":null}",
            "{\"toolbar\":[]}", "{\"toolbar\":{}}", "{\"toolbar\":{\"tabs\":null}}",
            "{\"toolbar\":{\"tabs\":{}}}", "{\"toolbar\":{\"tabs\":[null,3,{}]}}",
            R"({"toolbar":{"tabs":[{"label":3}]}})",
            R"({"toolbar":{"tabs":[{"label":""}]}})",
            R"({"toolbar":{"tabs":[{"label":"\u0000"}]}})",
            R"({"toolbar":{"tabs":[{"label":"Tab"}]}})",
            R"({"toolbar":{"tabs":[{"label":"Tab","groups":{}}]}})",
            R"({"toolbar":{"tabs":[{"label":"Tab","groups":[null,3,{}]}]}})"}) {
        SCOPED_TRACE(json);
        rapidjson::Document input;
        ASSERT_FALSE(input.Parse(json).HasParseError());
        customize.readData(input);
        auto output = generated();
        ASSERT_TRUE(output["toolbar"]["tabs"].IsArray());
    }
}

TEST_F(UiSettingsTest, ConfiguredDefaultDirectoryPrecedesBundledDefaults) {
    QFile file(QDir(directory.path()).filePath("default_ui_settings.json"));
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    ASSERT_GT(file.write(R"({"toolbar":{"tabs":[{"label":"Configured","groups":[]}]}})"), 0);
    file.close();
    for (bool defaults : {false, true}) {
        settings.readSettings(&customize, defaults);
        auto output = generated();
        ASSERT_EQ(output["toolbar"]["tabs"].Size(), 1u);
        EXPECT_STREQ(output["toolbar"]["tabs"][0]["label"].GetString(), "Configured");
    }
}

TEST_F(UiSettingsTest, InvalidWidthsUseSafeDefaultBeforeModelAllocation) {
    for (const char* width : {"0", "-1", "2.5", "null", "\"bad\"",
                              "2147483647", "-2147483648", "100"}) {
        SCOPED_TRACE(width);
        const std::string json = std::string(
            R"({"toolbar":{"tabs":[{"label":"Tab","groups":[{"label":"Group","width":)")
            + width + R"(,"buttons":[]}]}]}})";
        rapidjson::Document input;
        ASSERT_FALSE(input.Parse(json.c_str()).HasParseError());
        customize.readData(input);
        auto output = generated();
        ASSERT_EQ(output["toolbar"]["tabs"].Size(), 1u);
        ASSERT_EQ(output["toolbar"]["tabs"][0]["groups"].Size(), 1u);
        EXPECT_EQ(output["toolbar"]["tabs"][0]["groups"][0]["width"].GetInt(), 3);
    }
}

TEST_F(UiSettingsTest, PartialToolbarKeepsGoodTabsGroupsAndButtons) {
    toolbar.addTab("Source")->addGroup("Source")->addButton("Known", "");
    toolbar.generateButtonsMap();
    ASSERT_NE(toolbar.buttonByName("Known"), nullptr);
    rapidjson::Document input;
    ASSERT_FALSE(input.Parse(R"({"toolbar":{"tabs":[null,{"label":"Good","groups":[
        null,{"label":"Group","width":2,"buttons":[null,7,"Known"]}]}]}})").HasParseError());
    customize.readData(input);
    auto output = generated();
    ASSERT_EQ(output["toolbar"]["tabs"].Size(), 1u);
    const auto& group = output["toolbar"]["tabs"][0]["groups"][0];
    EXPECT_STREQ(group["label"].GetString(), "Group");
    EXPECT_EQ(group["width"].GetInt(), 2);
    ASSERT_EQ(group["buttons"].Size(), 1u);
    EXPECT_STREQ(group["buttons"][0].GetString(), "Known");
}

TEST_F(UiSettingsTest, RuntimeGroupsSurviveTheStartupSettingsRestore) {
    auto* quickAccess = toolbar.addTab("Quick Access");
    quickAccess->addGroup("Creation")->addButton("Line", "");
    toolbar.addButton("PythonPluginOperation", "", "Creation",
                      lc::scripting::nativeCallback([] {}),
                      "Plugin operation", false, "Quick Access", nullptr, true);

    // Simulates a plugin that contributed its own toolbar group while the
    // application was starting; the settings document knows nothing about it.
    auto* pluginGroup = quickAccess->addGroup("Plugin");
    pluginGroup->addButton("Rectangle", "");

    // A plugin may also contribute a whole tab that the settings document
    // knows nothing about.
    auto* pluginTab = toolbar.addTab("Plugin Tab");
    pluginTab->addGroup("Extra")->addButton("Gear", "");
    toolbar.generateButtonsMap();

    rapidjson::Document input;
    ASSERT_FALSE(input.Parse(R"({"toolbar":{"tabs":[{"label":"Quick Access","groups":[
        {"label":"Creation","width":3,"buttons":["Line"]}]}]}})").HasParseError());

    customize.readData(input);
    customize.setCloseMode(lc::ui::widgets::CustomizeToolbar::CloseMode::Save);
    customize.setPreserveRuntimeGroups(true);
    customize.close();

    auto* restoredTab = toolbar.tabByName("Quick Access");
    ASSERT_NE(nullptr, restoredTab);

    auto* restoredManagedGroup = restoredTab->groupByName("Creation");
    ASSERT_NE(nullptr, restoredManagedGroup);
    EXPECT_NE(nullptr, restoredManagedGroup->buttonByName("Line"));
    EXPECT_NE(nullptr, restoredManagedGroup->buttonByName("PythonPluginOperation"));

    auto* restoredPluginGroup = restoredTab->groupByName("Plugin");
    ASSERT_NE(nullptr, restoredPluginGroup);
    EXPECT_NE(nullptr, restoredPluginGroup->buttonByName("Rectangle"));

    auto* restoredPluginTab = toolbar.tabByName("Plugin Tab");
    ASSERT_NE(nullptr, restoredPluginTab);
    auto* restoredExtraGroup = restoredPluginTab->groupByName("Extra");
    ASSERT_NE(nullptr, restoredExtraGroup);
    EXPECT_NE(nullptr, restoredExtraGroup->buttonByName("Gear"));
}

TEST_F(UiSettingsTest, ManualCustomizationStillRemovesGroupsMissingFromTheModel) {
    auto* quickAccess = toolbar.addTab("Quick Access");
    auto* removedGroup = quickAccess->addGroup("Removed");
    removedGroup->addButton("Old", "");
    toolbar.generateButtonsMap();

    rapidjson::Document input;
    ASSERT_FALSE(input.Parse(R"({"toolbar":{"tabs":[{"label":"Quick Access","groups":[]}]}})").HasParseError());

    customize.readData(input);
    customize.setCloseMode(lc::ui::widgets::CustomizeToolbar::CloseMode::Save);
    customize.close();

    auto* restoredTab = toolbar.tabByName("Quick Access");
    ASSERT_NE(nullptr, restoredTab);
    EXPECT_EQ(nullptr, restoredTab->groupByName("Removed"));
}

TEST_F(UiSettingsTest, ManualCustomizationStillRemovesRuntimeButtonsOmittedFromAManagedGroup) {
    auto* quickAccess = toolbar.addTab("Quick Access");
    quickAccess->addGroup("Creation")->addButton("Line", "");
    toolbar.addButton("PluginOperation", "", "Creation",
                      lc::scripting::nativeCallback([] {}),
                      "Plugin operation", false, "Quick Access", nullptr, true);
    toolbar.generateButtonsMap();

    rapidjson::Document input;
    ASSERT_FALSE(input.Parse(R"({"toolbar":{"tabs":[{"label":"Quick Access","groups":[
        {"label":"Creation","width":3,"buttons":["Line"]}]}]}})").HasParseError());

    customize.readData(input);
    customize.setCloseMode(lc::ui::widgets::CustomizeToolbar::CloseMode::Save);
    customize.close();

    auto* restoredTab = toolbar.tabByName("Quick Access");
    ASSERT_NE(nullptr, restoredTab);
    auto* restoredGroup = restoredTab->groupByName("Creation");
    ASSERT_NE(nullptr, restoredGroup);
    EXPECT_NE(nullptr, restoredGroup->buttonByName("Line"));
    EXPECT_EQ(nullptr, restoredGroup->buttonByName("PluginOperation"));
}

TEST_F(UiSettingsTest, RuntimeRemovalsSurviveRestartAndCanBeReadded) {
    auto populate = [](lc::ui::widgets::Toolbar& target, bool newPlugin) {
        auto* tab = target.addTab("Quick Access");
        tab->addGroup("Creation")->addButton("Line", "");
        target.addButton("PluginOperation", "", "Creation", lc::scripting::nativeCallback([] {}),
                         "", false, "Quick Access", nullptr, true);
        tab->addGroup("Plugin")->addButton("Rectangle", "")->setPreserveOnUiSettingsRestore(true);
        target.addTab("Plugin Tab")->addGroup("Extra")->addButton("Gear", "")->setPreserveOnUiSettingsRestore(true);
        if (newPlugin) {
            target.addButton("NewOperation", "", "Creation", lc::scripting::nativeCallback([] {}),
                             "", false, "Quick Access", nullptr, true);
            target.addButton("NewGroupOperation", "", "Plugin", lc::scripting::nativeCallback([] {}),
                             "", false, "Quick Access", nullptr, true);
            target.addButton("NewGroupOperation2", "", "Plugin", lc::scripting::nativeCallback([] {}),
                             "", false, "Quick Access", nullptr, true);
            tab->addGroup("New Plugin")->addButton("NewButton", "");
        }
        target.generateButtonsMap();
    };
    auto restore = [this](lc::ui::widgets::Toolbar& target) {
        lc::ui::widgets::CustomizeToolbar startup(&target);
        settings.readSettings(&startup);
        startup.setPreserveRuntimeGroups(true);
        startup.setCloseMode(lc::ui::widgets::CustomizeToolbar::CloseMode::Save);
        startup.close();
    };

    lc::ui::widgets::Toolbar first(nullptr);
    populate(first, false);
    lc::ui::widgets::CustomizeToolbar edit(&first);
    rapidjson::Document edited;
    ASSERT_FALSE(edited.Parse(R"({"toolbar":{"tabs":[{"label":"Quick Access","groups":[
        {"label":"Creation","width":3,"buttons":["Line"]}]}]}})").HasParseError());
    edit.readData(edited);
    edit.setCloseMode(lc::ui::widgets::CustomizeToolbar::CloseMode::Save);
    edit.close();
    settings.writeSettings(&edit);

    lc::ui::widgets::Toolbar noNewPlugin(nullptr);
    populate(noNewPlugin, false);
    restore(noNewPlugin);
    ASSERT_NE(nullptr, noNewPlugin.tabByName("Quick Access"));
    EXPECT_EQ(nullptr, noNewPlugin.tabByName("Quick Access")->groupByName("Plugin"));

    lc::ui::widgets::Toolbar second(nullptr);
    populate(second, true);
    restore(second);
    auto* tab = second.tabByName("Quick Access");
    ASSERT_NE(nullptr, tab);
    auto* creation = tab->groupByName("Creation");
    ASSERT_NE(nullptr, creation);
    EXPECT_NE(nullptr, creation->buttonByName("Line"));
    EXPECT_EQ(nullptr, creation->buttonByName("PluginOperation"));
    EXPECT_NE(nullptr, creation->buttonByName("NewOperation"));
    auto* plugin = tab->groupByName("Plugin");
    ASSERT_NE(nullptr, plugin);
    EXPECT_EQ(nullptr, plugin->buttonByName("Rectangle"));
    ASSERT_NE(nullptr, plugin->buttonByName("NewGroupOperation"));
    ASSERT_NE(nullptr, plugin->buttonByName("NewGroupOperation2"));
    auto* grid = qobject_cast<QGridLayout*>(plugin->layout());
    ASSERT_NE(nullptr, grid);
    ASSERT_NE(nullptr, grid->itemAtPosition(0, 0));
    ASSERT_NE(nullptr, grid->itemAtPosition(0, 1));
    EXPECT_EQ(plugin->buttonByName("NewGroupOperation"), grid->itemAtPosition(0, 0)->widget());
    EXPECT_EQ(plugin->buttonByName("NewGroupOperation2"), grid->itemAtPosition(0, 1)->widget());
    auto* inserted = plugin->addButton("InsertedAfterRestore", "");
    ASSERT_NE(nullptr, grid->itemAtPosition(0, 2));
    EXPECT_EQ(inserted, grid->itemAtPosition(0, 2)->widget());
    plugin->removeButton(inserted);
    EXPECT_NE(nullptr, tab->groupByName("New Plugin"));
    EXPECT_EQ(nullptr, second.tabByName("Plugin Tab"));

    // Saving an unrelated later edit must retain earlier removal records.
    lc::ui::widgets::CustomizeToolbar unrelated(&second);
    settings.writeSettings(&unrelated);
    lc::ui::widgets::Toolbar third(nullptr);
    populate(third, true);
    restore(third);
    tab = third.tabByName("Quick Access");
    ASSERT_NE(nullptr, tab);
    ASSERT_NE(nullptr, tab->groupByName("Creation"));
    EXPECT_EQ(nullptr, tab->groupByName("Creation")->buttonByName("PluginOperation"));
    ASSERT_NE(nullptr, tab->groupByName("Plugin"));
    EXPECT_EQ(nullptr, tab->groupByName("Plugin")->buttonByName("Rectangle"));
    EXPECT_NE(nullptr, tab->groupByName("Plugin")->buttonByName("NewGroupOperation"));
    EXPECT_EQ(nullptr, third.tabByName("Plugin Tab"));

    lc::ui::widgets::CustomizeToolbar readd(&third);
    rapidjson::Document restoredLayout;
    ASSERT_FALSE(restoredLayout.Parse(R"({"toolbar":{"tabs":[{"label":"Quick Access","groups":[
        {"label":"Creation","width":3,"buttons":["Line","PluginOperation"]},
        {"label":"Plugin","width":3,"buttons":["Rectangle"]}]},
        {"label":"Plugin Tab","groups":[{"label":"Extra","width":3,"buttons":["Gear"]}]}]}})").HasParseError());
    readd.readData(restoredLayout);
    readd.setCloseMode(lc::ui::widgets::CustomizeToolbar::CloseMode::Save);
    readd.close();
    settings.writeSettings(&readd);
    lc::ui::widgets::Toolbar fourth(nullptr);
    populate(fourth, false);
    restore(fourth);
    tab = fourth.tabByName("Quick Access");
    ASSERT_NE(nullptr, tab);
    ASSERT_NE(nullptr, tab->groupByName("Creation"));
    EXPECT_NE(nullptr, tab->groupByName("Creation")->buttonByName("PluginOperation"));
    ASSERT_NE(nullptr, tab->groupByName("Plugin"));
    EXPECT_NE(nullptr, tab->groupByName("Plugin")->buttonByName("Rectangle"));
    EXPECT_NE(nullptr, fourth.tabByName("Plugin Tab"));
}

TEST_F(UiSettingsTest, LegacyAndMalformedRemovalMetadataPreservesNewPlugins) {
    for (const char* metadata : {"", R"(,"toolbarRemovedGroups":42,"toolbarRemovedRuntimeButtons":[null,[1,2,3]])"}) {
        SCOPED_TRACE(metadata);
        lc::ui::widgets::Toolbar target(nullptr);
        target.addTab("Quick Access")->addGroup("Creation")->addButton("Line", "");
        target.tabByName("Quick Access")->groupByName("Creation")->addButton("Unwanted", "");
        target.addButton("PluginOperation", "", "Creation", lc::scripting::nativeCallback([] {}),
                         "", false, "Quick Access", nullptr, true);
        target.tabByName("Quick Access")->addGroup("Plugin")->addButton("Rectangle", "");
        target.generateButtonsMap();
        const std::string json = std::string(R"({"toolbar":{"tabs":[{"label":"Quick Access","groups":[{"label":"Creation","buttons":["Line"]}]}]})") + metadata + "}";
        rapidjson::Document input;
        ASSERT_FALSE(input.Parse(json.c_str()).HasParseError());
        writeSettingsFile(QByteArray::fromStdString(json));
        lc::ui::widgets::CustomizeToolbar startup(&target);
        settings.readSettings(&startup);
        startup.setPreserveRuntimeGroups(true);
        startup.setCloseMode(lc::ui::widgets::CustomizeToolbar::CloseMode::Save);
        startup.close();
        auto* tab = target.tabByName("Quick Access");
        ASSERT_NE(nullptr, tab);
        ASSERT_NE(nullptr, tab->groupByName("Creation"));
        EXPECT_EQ(nullptr, tab->groupByName("Creation")->buttonByName("Unwanted"));
        EXPECT_NE(nullptr, tab->groupByName("Creation")->buttonByName("PluginOperation"));
        EXPECT_NE(nullptr, tab->groupByName("Plugin"));
    }
}

TEST_F(UiSettingsTest, MovedRuntimeButtonStaysOnlyInItsSavedGroupAfterRestart) {
    auto populate = [](lc::ui::widgets::Toolbar& target) {
        auto* tab = target.addTab("Quick Access");
        tab->addGroup("Creation")->addButton("Line", "");
        tab->addGroup("Tools")->addButton("Other", "");
        target.addButton("PluginOperation", "", "Creation", lc::scripting::nativeCallback([] {}),
                         "", false, "Quick Access", nullptr, true);
        target.generateButtonsMap();
    };
    auto restore = [this](lc::ui::widgets::Toolbar& target) {
        lc::ui::widgets::CustomizeToolbar startup(&target);
        settings.readSettings(&startup);
        startup.setPreserveRuntimeGroups(true);
        startup.setCloseMode(lc::ui::widgets::CustomizeToolbar::CloseMode::Save);
        startup.close();
    };
    auto expectMoved = [](lc::ui::widgets::Toolbar& target) {
        auto* tab = target.tabByName("Quick Access");
        ASSERT_NE(nullptr, tab);
        auto* creation = tab->groupByName("Creation");
        auto* tools = tab->groupByName("Tools");
        ASSERT_NE(nullptr, creation);
        ASSERT_NE(nullptr, tools);
        EXPECT_EQ(nullptr, creation->buttonByName("PluginOperation"));
        EXPECT_NE(nullptr, tools->buttonByName("PluginOperation"));
    };

    lc::ui::widgets::Toolbar first(nullptr);
    populate(first);
    lc::ui::widgets::CustomizeToolbar edit(&first);
    rapidjson::Document moved;
    ASSERT_FALSE(moved.Parse(R"({"toolbar":{"tabs":[{"label":"Quick Access","groups":[
        {"label":"Creation","width":3,"buttons":["Line"]},
        {"label":"Tools","width":3,"buttons":["Other","PluginOperation"]}]}]}})").HasParseError());
    edit.readData(moved);
    edit.setCloseMode(lc::ui::widgets::CustomizeToolbar::CloseMode::Save);
    edit.close();
    settings.writeSettings(&edit);
    expectMoved(first);

    lc::ui::widgets::Toolbar second(nullptr);
    populate(second);
    restore(second);
    expectMoved(second);
    lc::ui::widgets::CustomizeToolbar unrelated(&second);
    settings.writeSettings(&unrelated);
    lc::ui::widgets::Toolbar third(nullptr);
    populate(third);
    restore(third);
    expectMoved(third);
}

TEST_F(UiSettingsTest, RestoringDefaultsClearsRuntimeRemovalHistory) {
    const QByteArray layout = R"({"toolbar":{"tabs":[{"label":"Quick Access","groups":[
        {"label":"Creation","buttons":["Line"]}]}]}})";
    QFile defaults(QDir(directory.path()).filePath("default_ui_settings.json"));
    ASSERT_TRUE(defaults.open(QIODevice::WriteOnly));
    ASSERT_EQ(defaults.write(layout), layout.size());
    defaults.close();
    writeSettingsFile(R"({"toolbar":{"tabs":[{"label":"Quick Access","groups":[
        {"label":"Creation","buttons":["Line"]}]}]},
        "toolbarRemovedRuntimeButtons":[["Quick Access","Creation","PluginOperation"]]})");

    auto populate = [](lc::ui::widgets::Toolbar& target) {
        target.addTab("Quick Access")->addGroup("Creation")->addButton("Line", "");
        target.addButton("PluginOperation", "", "Creation", lc::scripting::nativeCallback([] {}),
                         "", false, "Quick Access", nullptr, true);
        target.generateButtonsMap();
    };
    auto restore = [this](lc::ui::widgets::Toolbar& target) {
        lc::ui::widgets::CustomizeToolbar startup(&target);
        settings.readSettings(&startup);
        startup.setPreserveRuntimeGroups(true);
        startup.setCloseMode(lc::ui::widgets::CustomizeToolbar::CloseMode::Save);
        startup.close();
    };
    lc::ui::widgets::Toolbar first(nullptr);
    populate(first);
    restore(first);
    ASSERT_EQ(nullptr, first.tabByName("Quick Access")->groupByName("Creation")->buttonByName("PluginOperation"));

    lc::ui::widgets::CustomizeToolbar edit(&first);
    QObject::connect(&edit, &lc::ui::widgets::CustomizeToolbar::defaultSettingsLoad,
                     [&] { settings.readSettings(&edit, true); });
    edit.defaultButtonClicked();
    edit.setCloseMode(lc::ui::widgets::CustomizeToolbar::CloseMode::Save);
    edit.close();
    settings.writeSettings(&edit);

    lc::ui::widgets::Toolbar second(nullptr);
    populate(second);
    restore(second);
    auto* tab = second.tabByName("Quick Access");
    ASSERT_NE(nullptr, tab);
    ASSERT_NE(nullptr, tab->groupByName("Creation"));
    EXPECT_NE(nullptr, tab->groupByName("Creation")->buttonByName("PluginOperation"));
}

TEST_F(UiSettingsTest, InvalidSettingsFilesCanBeRewrittenAsObjects) {
    for (const char* json : {"null", "[]", "{", "{}", "{\"toolbar\":42}"}) {
        SCOPED_TRACE(json);
        writeSettingsFile(json);
        std::map<std::string, int> proportions{{"CliCommand", 35}};
        settings.writeDockSettings({{"CliCommand", 8}}, proportions);
        auto output = readSettingsFile();
        ASSERT_TRUE(output.IsObject());
        ASSERT_TRUE(output.HasMember("dockPositions"));
        ASSERT_EQ(output["dockPositions"].Size(), 1u);
        EXPECT_EQ(output["dockPositions"][0]["proportion"].GetInt(), 35);
    }
}

TEST_F(UiSettingsTest, InvalidDockEntriesAreIgnoredAndValidEntriesSurvive) {
    writeSettingsFile(R"({"toolbar":{"tabs":[]},"dockPositions":[null,7,{},
        {"widget":3,"position":8},{"widget":"Bad","position":"8"},
        {"widget":"CliCommand","position":8,"proportion":35},
        {"widget":"Layers","position":2,"proportion":"bad"}]})");
    std::map<std::string, int> proportions;
    const auto positions = settings.readDockSettings(proportions);
    EXPECT_EQ(positions, (std::map<std::string, int>{{"CliCommand", 8}, {"Layers", 2}}));
    EXPECT_EQ(proportions, (std::map<std::string, int>{{"CliCommand", 35}}));
}

TEST_F(UiSettingsTest, WritingToolbarPreservesExistingDockSettings) {
    writeSettingsFile(R"({"toolbar":{"tabs":[]},"dockPositions":[
        {"widget":"CliCommand","position":8,"proportion":35}]})");
    settings.readSettings(&customize);
    settings.writeSettings(&customize);
    auto output = readSettingsFile();
    ASSERT_TRUE(output.IsObject());
    ASSERT_TRUE(output["toolbar"]["tabs"].IsArray());
    EXPECT_EQ(output["dockPositions"][0]["proportion"].GetInt(), 35);
}
