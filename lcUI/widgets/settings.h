#pragma once

#include <memory>
#include <unordered_map>

class Settings
{
	public:
		Settings(){
			Settings::variable_map["LCTOLERANCE"] = 1.0e-10;
		};


		static double getVal(std::string variable);

		static void setVal(std::string variable,double val);

		static std::unordered_map<std::string, double> variable_map;

		static void instance();
};