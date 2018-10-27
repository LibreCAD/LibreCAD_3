#pragma once
#define BOOST_LOG_DYN_LINK
#include <boost/log/common.hpp>
#include <boost/log/sources/logger.hpp>

namespace src = boost::log::sources;

enum severity_level {
    normal,
    notification,
    warning,
    error,
    critical
};

namespace lc {
    namespace log {
        typedef src::severity_logger<severity_level> lc_logger;
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
