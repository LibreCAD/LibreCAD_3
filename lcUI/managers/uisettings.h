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

        private:
            const QString settingsFileName = "ui_settings.xml";
        };
    }
}
