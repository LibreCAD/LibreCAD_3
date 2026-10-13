#include <cad/storage/settings/modulesettings.h>
#include <widgets/customizeToolbar/customizetoolbar.h>

namespace lc
{
namespace ui
{
/**
* \brief UI settings
*/
class UiSettings : public lc::storage::settings::ModuleSettings
{
public:
    /**
    * \brief Write the ui settings to settings file
    * \param pointer to CustomizeToolbar widget
    */
    void writeSettings(widgets::CustomizeToolbar* customizeToolbar);

    /**
    * \brief Read ui settings (called by mainwindow)
    * \param pointer to CustomizeToolbar widget
    * \param bool defaultSettings read from default settings
    */
    void readSettings(widgets::CustomizeToolbar* customizeToolbar, bool defaultSettings = false);

    /**
    * \brief Write the dock settings to the document file
    */
    void writeDockSettings(const std::map<std::string, int>& positions, std::map<std::string, int>& proportions);

    /**
    * \brief Read the dock settings
    */
    std::map<std::string, int> readDockSettings(std::map<std::string, int>& proportions);

private:
    /**
    * \brief Validates with schema file
    * \param input document
    * \return bool, true if validates or any problem with schema document, else returns false
    */
    bool validateSettingsDocument(rapidjson::Document& inputDocument);

    /**
    * \brief Get the settings document by reading the given file
    *
    * Falls back on the default settings file and then on the settings built into
    * the binary. The returned document is always a json object, possibly empty:
    * callers may use HasMember()/AddMember() on it without first checking, which
    * rapidjson would otherwise assert on.
    */
    rapidjson::Document getSettingsDocument(std::string fileName);

    /**
    * \brief Write the settings document to the settings file
    */
    void writeSettingsFile(rapidjson::Document& document);

private:
    const std::string settingsFileName = "ui_settings.json";
    const std::string defaultSettingsFileName = "default_ui_settings.json";
    const std::string schemaFileName = "settings_schema.json";
};
}
}
