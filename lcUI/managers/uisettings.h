#include <cad/storage/settings/modulesettings.h>
#include <widgets/customizeToolbar/customizetoolbar.h>

namespace lc
{
    namespace ui
    {
        class UiSettings : public lc::storage::settings::ModuleSettings
        {
        public:
            void writeSettings(widgets::CustomizeToolbar* customizeToolbar);

            void readSettings(widgets::CustomizeToolbar* customizeToolbar, bool defaultSettings = false);

        private:
            const QString settingsFileName = "ui_settings.xml";
            const QString defaultSettingsFileName = "default_ui_settings.xml";
        };
    }
}
