#pragma once
#include <boost/log/common.hpp>
#include <boost/log/sources/logger.hpp>

namespace src = boost::log::sources;

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(lcGlobalLogger, src::logger_mt)

namespace lc {
    namespace log {
        enum SeverityLevel {
            TRACE, // Verbose information
            DEBUG, // Internal errors
            INFO, // Information
            WARNING, // Recovered errors, that may produce error in the drawing or the program after
            ERROR, // Fatal errors that interrupt the program or the current process
        };

        class Logger {
            public:
                static Logger* Instance();

            private:
                void enableFileSink();
                void enableConsoleSink();
                Logger();
                Logger(Logger const&)=delete;
                Logger& operator=(Logger const&)=delete;
                static Logger* instance;
        };
    }
}

#define LOGGER lc::log::Logger::Instance()

#define LOG_TRACE BOOST_LOG_SEV(lcGlobalLogger::get(), lc::log::TRACE)
#define LOG_DEBUG BOOST_LOG_SEV(lcGlobalLogger::get(), lc::log::DEBUG)
#define LOG_INFO BOOST_LOG_SEV(lcGlobalLogger::get(), lc::log::INFO)
#define LOG_WARNING BOOST_LOG_SEV(lcGlobalLogger::get(), lc::log::WARNING)
#define LOG_ERROR BOOST_LOG_SEV(lcGlobalLogger::get(), lc::log::ERROR)
