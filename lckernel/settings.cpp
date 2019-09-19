#include <settings.h>

using namespace lc::storage::settings;

auto lcTolerance = DoubleSettingValue(1.0e-10);

ModuleSettings settings({
        {SETTINGS_LCTOLERANCE, &lcTolerance}
});

ModuleSettings lc::kernel::getSettings() {
	return settings;
}