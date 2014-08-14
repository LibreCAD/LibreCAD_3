#pragma once

#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/helpers/exception.h"

namespace lc {
    const log4cxx::LoggerPtr logger = log4cxx::Logger::getRootLogger();
}

#define TOLERANCE 1.0e-10
//squared tolerance
#define TOLERANCE15 1.5e-15
#define TOLERANCE2 1.0e-20
#define TOLERANCE_ANGLE 1.0e-8
