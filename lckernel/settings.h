#pragma once

#include <cad/storage/settings/modulesettings.h>
#define SETTINGS_LCTOLERANCE "Tolerance"

namespace lc {
    namespace kernel {
        lc::storage::settings::ModuleSettings getSettings();
    }
}