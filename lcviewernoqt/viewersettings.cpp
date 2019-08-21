#include "viewersettings.h"
#include "res.h"
using namespace lc::storage::settings;

auto shaderpath = StringSettingValue(lc::viewer::opengl::SHADER_PATH);
auto fontpath = StringSettingValue(lc::viewer::opengl::FONT_PATH);


ModuleSettings lc::viewer::resourcesPathSettings({
        {SETTINGS_GL_SHADER_PATH, &shaderpath},
        {SETTINGS_GL_FONT_PATH, &fontpath}
});

