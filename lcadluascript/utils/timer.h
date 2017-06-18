#pragma once

#ifdef _WIN32
#include <Windows.h>
#define EXPORT_DECL __declspec(dllexport)
#else
#include <time.h>
#include <sys/time.h>
#define EXPORT_DECL
#endif

#ifdef _WIN32
static double timer_resolution = 0.0;
#endif

static double lua_microtime() {
    double time = -1.;
#ifdef _WIN32
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    time = (lua_Number)(counter.QuadPart * timer_resolution);
#elif _POSIX_TIMERS > 0 && defined(_POSIX_MONOTONIC_CLOCK) && defined(CLOCK_MONOTONIC_RAW) || defined(CLOCK_MONOTONIC)
    struct timespec t;
#ifdef CLOCK_MONOTONIC_RAW
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
#else
    clock_gettime(CLOCK_MONOTONIC, &t);
#endif
    time = ((double)t.tv_sec + (double)t.tv_nsec / 1000000000.0);
#else
    struct timeval t;
    gettimeofday(&t, nullptr);
    time = ((double)t.tv_sec + (double)t.tv_usec / 1000000.0);
#endif

    return time;
}
