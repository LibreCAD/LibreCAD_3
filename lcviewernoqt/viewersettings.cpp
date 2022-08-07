#include "viewersettings.h"
#include <painters/opengl/resources/res.h>
using namespace lc::storage::settings;

auto shaderpath = StringSettingValue(lc::viewer::opengl::SHADER_PATH);
auto fontpath = StringSettingValue(lc::viewer::opengl::FONT_PATH);

void lc::viewer::setShader(lc::storage::settings::StringSettingValue pathToShaders) {
    lc::viewer::viewerSettings.set(SETTINGS_GL_SHADER_PATH, pathToShaders);
}

void lc::viewer::setFont(lc::storage::settings::StringSettingValue pathToFonts) {
    lc::viewer::viewerSettings.set(SETTINGS_GL_FONT_PATH, pathToFonts);
}


ModuleSettings lc::viewer::viewerSettings({
    {SETTINGS_GL_SHADER_PATH, &shaderpath},
    {SETTINGS_GL_FONT_PATH, &fontpath}
    });
