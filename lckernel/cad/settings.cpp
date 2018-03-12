#include "settings.h"
#include <memory>
#include <unordered_map>

Settings* Settings::inst = nullptr;
std::unordered_map<std::string, double> Settings::variable_map;

void Settings::setVal(std::string variable,double val)
{
	variable_map[variable] = val;
}

double Settings::getVal(std::string variable)
{
	Settings::inst = Settings::instance();

	return variable_map[variable];
}

Settings* Settings::instance()
{
	if(Settings::inst==nullptr)
		Settings::inst = new Settings();

	return Settings::inst;
}
