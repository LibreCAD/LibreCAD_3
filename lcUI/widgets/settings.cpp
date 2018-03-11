#include "settings.h"
#include <memory>
#include <unordered_map>

Settings* inst = nullptr;

std::unordered_map<std::string, double> Settings::variable_map;

void Settings::setVal(std::string variable,double val)
{
	variable_map[variable] = val;
}

double Settings::getVal(std::string variable)
{
	Settings::instance();
	return variable_map[variable];
}

void Settings::instance()
{
	if(inst==nullptr)
		printf("works\n");
		inst = new Settings();
}