#pragma once

#include <string>
#include <memory>
#include <unordered_map>

class Settings;

class Settings
{
	public:
		static double val(const std::string& variable);

		static void setVal(const std::string& variable,double val);

		static bool exists(const std::string& variable);

	private:
		static std::unordered_map<std::string, double> _variable_map;
};
