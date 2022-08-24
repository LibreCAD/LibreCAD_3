#pragma once

#include <cad/storage/settings/modulesettings.h>
#define SETTINGS_GL_SHADER_PATH "shaderpath"
#define SETTINGS_GL_FONT_PATH "fontpath"

namespace lc {
    namespace viewer {
        extern lc::storage::settings::ModuleSettings viewerSettings;

        #ifndef VIEWERSETTINGS_SETTERS
        #define VIEWERSETTINGS_SETTERS
        void setShader(lc::storage::settings::StringSettingValue pathToShaders);

        void setFont(lc::storage::settings::StringSettingValue pathToFonts);
        #endif //VIEWERSETTINGS_SETTERS
    }
}
