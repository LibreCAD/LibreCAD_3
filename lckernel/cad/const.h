#pragma once

#include <assert.h>

#define LCTOLERANCE 1.0e-10
#define LCARCTOLERANCE 1.0e-10
#define BBHEURISTIC 1.0e-5
//squared tolerance
#define TOLERANCE15 1.5e-15
#define TOLERANCE2 1.0e-20
#define TOLERANCE_ANGLE 1.0e-8

template<typename T>
T * ptr(T & obj) { return &obj; } //turn reference into pointer!

template<typename T>
T * ptr(T * obj) { return obj; } //obj is already pointer, return it!


namespace lc {
enum class Units {
    Inch,
    Foot,
    Mile,
    Millimeter,
    Centimeter,
    Meter,
    Kilometer,
    Microinch,
    Mil,
    Yard,
    Angstrom,
    Nanometer,
    Micron,
    Decimeter,
    Decameter,
    Hectometer,
    Gigameter,
    Astro,
    Lightyear,
    Parsec,
    None
};

enum class AngleFormat {
    DegreesDecimal,
    DegreesMinutesSeconds,
    Gradians,
    Radians,
    Surveyors
};

enum class LineType {
LineByLayer,
LineByBlock,
DotLine,
DotLineTiny,
DotLine2,
DotLineX2,
DashLine,
DashLineTiny,
DashLine2,
DashLineX2,
DashDotLine,
DashDotLineTiny,
DashDotLine2,
DashDotLineX2,
DivideLine,
DivideLineTiny,
DivideLine2,
DivideLineX2,
CenterLine,
CenterLineTiny,
CenterLine2,
CenterLineX2,
BorderLine,
BorderLineTiny,
BorderLine2,
BorderLineX2,
SolidLine
};

}

