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

        private:
            const QString settingsFileName = "ui_settings.xml";
            const QString defaultSettingsFileName = "default_ui_settings.xml";
        };
    }
}
