#pragma once

#include <memory>
#include <unordered_map>

class Settings;

class Settings
{
	public:
		Settings() {
			variable_map["LCTOLERANCE"] = 1.0e-10;
		};

		static Settings* inst;

		static double getVal(const std::string& variable);

		static void setVal(const std::string& variable,double val);

		static Settings* instance();

	private:
		static std::unordered_map<std::string, double> variable_map;
};
