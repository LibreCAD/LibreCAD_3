#pragma once

#include <assert.h>
#include <../lcUI/widgets/settings.h>

#define LCTOLERANCE Settings::getVal("LCTOLERANCE")
#define LCARCTOLERANCE 1.0e-10
#define BBHEURISTIC 1.0e-5
#define BBHEURISTIC2 1.0e-3
//squared tolerance
#define TOLERANCE15 1.5e-15
#define TOLERANCE2 1.0e-20
#define TOLERANCE_ANGLE 1.0e-8

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DECLARE_SHORT_SHARED_PTR(type)              \
    using type##_SPtr = std::shared_ptr<type>;        \
    using type##_CSPtr = std::shared_ptr<const type>;

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

}

