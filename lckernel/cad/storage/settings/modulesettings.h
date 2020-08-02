#pragma once

#include <string>
#include <map>
#include "doublesettingvalue.h"
#include "stringsettingvalue.h"

namespace lc {
    namespace storage {
        namespace settings {
            class ModuleSettings {
                public:
                    /**
                     * @brief Construct an empty ModuleSettings
                     */
                    ModuleSettings();

                    /**
                     * @brief Construct a ModuleSettings with the given settings
                     * @param settings Settings
                     */
                    ModuleSettings(std::initializer_list<std::pair<std::string, ISettingValue*>> settings);

                    virtual ~ModuleSettings();

                    /**
                     * @brief Return setting value
                     * @param name Name of the setting
                     * @return Value
                     */
                    const ISettingValue* get(const std::string& name) const;

                    /**
                     * @brief Create or update a setting
                     * @param name Name of the setting
                     * @param value Value of the setting
                     * For safety reasons, a setting type can't be changed. If a setting is already defined, its
                     * setValue method will be called.
                     * @throw SettingTypeException
                     */
                    void set(const std::string& name, ISettingValue& value);

                protected:
                    std::map<std::string, std::string> _filePaths;

                private:
                    std::map<std::string, ISettingValue*> _settings;
            };
        }
    }
}
