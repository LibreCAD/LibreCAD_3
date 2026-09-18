#pragma once

#if _MSC_VER && !__INTEL_COMPILER
#define  _WIN32_WINNT 0x0601
#endif


#include <cstddef>
#include <ostream>

#include <boost/log/common.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/severity_logger.hpp>

namespace src = boost::log::sources;

namespace lc {
namespace log {
enum SeverityLevel {
    LOG_SEVERITY_TRACE, // Verbose information
    LOG_SEVERITY_DEBUG, // Internal errors
    LOG_SEVERITY_INFO, // Information
    LOG_SEVERITY_WARNING, // Recovered errors, that may produce error in the drawing or the program after
    LOG_SEVERITY_ERROR, // Fatal errors that interrupt the program or the current process
};

/**
 * Renders a severity as its name.  This has to be visible wherever a sink
 * formatter is instantiated -- ADL only searches lc::log, so a definition in
 * logger.cpp leaves every other translation unit printing the raw enum value.
 */
template< typename CharT, typename TraitsT >
inline std::basic_ostream< CharT, TraitsT >& operator<< (
    std::basic_ostream< CharT, TraitsT >& strm, SeverityLevel lvl) {
    static const char* const str[] = {
        "TRACE",
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
    };

    if (static_cast< std::size_t >(lvl) < (sizeof(str) / sizeof(*str))) {
        strm << str[lvl];
    } else {
        strm << static_cast< int >(lvl);
    }

    return strm;
}
}
}

/**
 * The global logger must carry the severity feature: with a plain src::logger_mt
 * the severity passed by BOOST_LOG_SEV is accepted as an unused named parameter
 * and silently discarded, so no record ever gets a "Severity" attribute and the
 * sinks below can neither format nor filter on it.
 */
BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(lcGlobalLogger, src::severity_logger_mt<lc::log::SeverityLevel>)

namespace lc {
namespace log {

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

#define LOG_TRACE BOOST_LOG_SEV(lcGlobalLogger::get(), lc::log::LOG_SEVERITY_TRACE)
#define LOG_DEBUG BOOST_LOG_SEV(lcGlobalLogger::get(), lc::log::LOG_SEVERITY_DEBUG)
#define LOG_INFO BOOST_LOG_SEV(lcGlobalLogger::get(), lc::log::LOG_SEVERITY_INFO)
#define LOG_WARNING BOOST_LOG_SEV(lcGlobalLogger::get(), lc::log::LOG_SEVERITY_WARNING)
#define LOG_ERROR BOOST_LOG_SEV(lcGlobalLogger::get(), lc::log::LOG_SEVERITY_ERROR)
