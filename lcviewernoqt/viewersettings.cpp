#include "viewersettings.h"
#include "res.h"
using namespace lc::storage::settings;

auto shaderpath = StringSettingValue(lc::viewer::opengl::SHADER_PATH);
auto fontpath = StringSettingValue(lc::viewer::opengl::FONT_PATH);


ModuleSettings lc::viewer::shaderPathSettings({
        {SETTINGS_GL_SHADER_PATH, &shaderpath}
});

ModuleSettings lc::viewer::fontPathSettings({
        {SETTINGS_GL_FONT_PATH, &fontpath}
});