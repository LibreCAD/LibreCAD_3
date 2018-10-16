#include "settings.h"

using namespace lc::storage;

std::unordered_map<std::string, double> Settings::_variable_map = { // NOLINT
    {"LCTOLERANCE", 1.0e-10}
};

void Settings::setVal(const std::string& variable,double val) {
	_variable_map[variable] = val;
}

double Settings::val(const std::string& variable) {
	return _variable_map[variable];
}

bool Settings::exists(const std::string& variable) {
    return _variable_map.find(variable) != _variable_map.end();
}
