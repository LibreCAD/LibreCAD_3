#include "settings.h"

double lctolerancev=1.0e-10;
void settings::changeVal(double val)
{
	lctolerancev = val;
}

double settings::tolerance()
{
	return lctolerancev;
}