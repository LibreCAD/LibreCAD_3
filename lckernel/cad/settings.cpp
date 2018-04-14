#include "settings.h"

Settings* Settings::inst = nullptr;
std::unordered_map<std::string, double> Settings::variable_map;

void Settings::setVal(const std::string& variable,double val) {
	variable_map[variable] = val;
}

double Settings::getVal(const std::string& variable) {
	Settings::inst = Settings::instance();

	return variable_map[variable];
}

Settings* Settings::instance() {
	if(inst == nullptr) {
		inst = new Settings();
	}

	return inst;
}
