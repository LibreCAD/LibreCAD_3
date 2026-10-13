#include <gtest/gtest.h>

#include <QApplication>
#include <QDir>
#include <QFile>
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
