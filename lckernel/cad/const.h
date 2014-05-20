#ifndef CONST_H
#define CONST_H

#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/helpers/exception.h"

namespace lc {
    const log4cxx::LoggerPtr logger = log4cxx::Logger::getRootLogger();
}

#define TOLERANCE 1.0e-4
//#define M_PI 3.14
#define TOLERANCE_ANGLE 1.0e-6

#endif
