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


