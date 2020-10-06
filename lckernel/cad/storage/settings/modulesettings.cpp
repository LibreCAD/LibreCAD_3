#include "modulesettings.h"
#include "unknownsettingexception.h"
#include <build_constants.h>
#include <iostream>

using namespace lc::storage::settings;

ModuleSettings::ModuleSettings() {
    _filePaths["settings_load"] = SETTINGS_PATH;
    _filePaths["settings_load"] = _filePaths["settings_load"] + "/";
}

ModuleSettings::ModuleSettings(std::initializer_list<std::pair<std::string, ISettingValue*>> settings) : ModuleSettings() {
    for(const auto& setting : settings) {
        _settings[setting.first] = setting.second->clone();
    }
}

ModuleSettings::~ModuleSettings() {
    for(const auto& setting : _settings) {
        delete setting.second;
    }

    _settings.clear();
}

const ISettingValue* ModuleSettings::get(const std::string& name) const {
    try {
        return _settings.at(name);
    }
    catch(const std::out_of_range& e) {
        LOG_WARNING << "Unable to find setting " << name << ".";
        throw UnknownSettingException("Unable to find setting " + name);
    }
}

void ModuleSettings::set(const std::string& name, ISettingValue& value) {
    try {
        auto setting = _settings.at(name);
        setting->setValue(value.getString());
    }
    catch(const std::out_of_range& e) {
        _settings[name] = value.clone();
    }
    catch(const SettingTypeException& e) {
        LOG_WARNING << "Unable to update setting " << name << ": " << e.what();
        throw e;
    }
}
