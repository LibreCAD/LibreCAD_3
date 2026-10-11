/*
 * All unit tests should be written in a new file and added to CMakeLists.txt.
 * Nothing should be included here except gtest.h to prevent recompilation of a single test taking too much time.
 */
#include <iostream>
#include <build_constants.h>
#include <gtest/gtest.h>
#ifdef LC_WITH_PYTHONSCRIPT
#include <lcpython.h>
#include <cstdlib>
#include <exception>
#include <string>

std::string lcUnitTestExecutablePath;

namespace {
class ScopedEnvironmentValue {
public:
    ScopedEnvironmentValue(const char* name, const char* value)
        : _name(name) {
        const char* previous = std::getenv(name);
        if (previous != nullptr) {
            _previous = previous;
            _hadPrevious = true;
        }
        _set(name, value);
    }

    ~ScopedEnvironmentValue() {
        if (_hadPrevious) {
            _set(_name.c_str(), _previous.c_str());
        } else {
#ifdef _WIN32
            _putenv_s(_name.c_str(), "");
#else
            unsetenv(_name.c_str());
#endif
        }
    }

private:
    static void _set(const char* name, const char* value) {
#ifdef _WIN32
        _putenv_s(name, value);
#else
        setenv(name, value, 1);
#endif
    }

    std::string _name;
    std::string _previous;
    bool _hadPrevious = false;
};
}

namespace {
int runPythonRuntimeProbe(int argc, char** argv) {
    if (argc < 3 || std::string(argv[1]) != "--python-runtime-probe") {
        return -1;
    }

    const std::string mode = argv[2];
    try {
        if (mode == "missing" && argc == 4) {
            lc::python::PythonInit::configureForApplication(argv[3]);
        } else if (mode == "repeat" && argc == 4) {
            lc::python::PythonInit::configureForApplication(argv[3]);
            lc::python::PythonInit::configureForApplication(argv[3]);
            return 0;
        } else if (mode == "conflict" && argc == 5) {
            lc::python::PythonInit::configureForApplication(argv[3]);
            lc::python::PythonInit::configureForApplication(argv[4]);
        } else if (mode == "late" && argc == 4) {
            lc::python::PythonInit::initialize();
            lc::python::PythonInit::configureForApplication(argv[3]);
        } else {
            std::cerr << "invalid Python runtime probe arguments" << std::endl;
            return 64;
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return 2;
    }
    return 0;
}
}
#endif

int main(int argc, char **argv) {
    std::cout << "LibreCAD v" << VERSION_MAJOR << "." << VERSION_MINOR <<
              " (" << BUILD_INFO << " built on " << BUILD_DATE << ")" << std::endl;
#ifdef LC_WITH_PYTHONSCRIPT
    lcUnitTestExecutablePath = argv[0];
    const int probeResult = runPythonRuntimeProbe(argc, argv);
    if (probeResult >= 0) {
        return probeResult;
    }
#endif
    ::testing::InitGoogleTest(&argc, argv);
#ifdef LC_WITH_PYTHONSCRIPT
    try {
        // Exercise the application runtime before tests can lazily initialize
        // Python through another path. Poison ambient settings to verify the
        // isolated PyConfig uses the provider selected by CMake.
        ScopedEnvironmentValue poisonHome("PYTHONHOME", "/__librecad_python_poison__");
        ScopedEnvironmentValue poisonPath("PYTHONPATH", "/__librecad_python_poison__");
        lc::python::PythonInit::configureForApplication();
        lc::python::PythonInit::initialize();
    } catch (const std::exception& e) {
        std::cerr << "Python runtime initialization failed: " << e.what() << std::endl;
        return 2;
    }
#endif
    return RUN_ALL_TESTS();
}
