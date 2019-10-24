#include "logger.h"

// Modified from
// https:://github.com/boostorg/log/blob/develop/example/advanced_usage/main.cpp

#include <iostream>
#include <fstream>
#if BOOST_VERSION >= 106100 //TODO fix version
	#include <boost/core/null_deleter.hpp>
#else
	#include <boost/log/utility/empty_deleter.hpp>
#endif
#include <boost/log/expressions.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/common.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

//namespaces alias as of documentation
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

//Class starts here..
//Singleton class
//Making Supports to console,plain logs
typedef sinks::synchronous_sink<sinks::text_ostream_backend> text_sink;

template< typename CharT, typename TraitsT >
inline std::basic_ostream< CharT, TraitsT >& operator<< (
    std::basic_ostream< CharT, TraitsT >& strm, lc::log::SeverityLevel lvl) {
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

namespace lc {
    namespace log {
        Logger* Logger::Instance() {
            if (!instance) {
                instance = new Logger;
            }

            return instance;
        }
        void Logger::enableFileSink() {
            //First sink.. detail ;; to file
            boost::shared_ptr<text_sink> pSink(new text_sink);
            text_sink::locked_backend_ptr pBackend = pSink->locked_backend();
            boost::shared_ptr<std::ofstream> pStream(new std::ofstream("sample.log"));
            pBackend->add_stream(pStream);
            logging::core::get()->add_sink(pSink);
            pSink->set_formatter(expr::stream
                                 << expr::attr<unsigned int>("LineID")
                                 << "[" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%d.%m.%y %H.%M.%S.%f")
                                 << "][" << expr::attr<SeverityLevel>("Severity")
                                 << "][" << expr::attr<boost::posix_time::time_duration>("Uptime")
                                 << "][" << expr::if_(expr::has_attr("Tag"))
                                 [
                                     expr::stream << expr::attr <std::string>("Tag")
                                     << "]["
                                 ]
                                 << expr::format_named_scope("Scope", keywords::format = "%n", keywords::iteration = expr::reverse) << "]"
                                 << expr::smessage);
        }
        void Logger::enableConsoleSink() {
            //Second sink.. brief ;; to file
            boost::shared_ptr<text_sink> pSink(new text_sink);
            text_sink::locked_backend_ptr pBackend = pSink->locked_backend();
            boost::shared_ptr<std::ostream> pStream(&std::clog, 
#if BOOST_VERSION >= 106100
            	boost::null_deleter()
#else
            	logging::empty_deleter()
#endif
            	);
            pBackend->add_stream(pStream);
            logging::core::get()->add_sink(pSink);
            pSink->set_formatter(expr::stream
                                 << "[" << expr::attr<SeverityLevel>("Severity")
                                 << "]: " << expr::if_(expr::has_attr("Tag"))
                                 [
                                     expr::stream << "[" << expr::attr <std::string>("Tag") << "]"
                                 ]
                                 << expr::smessage);
            //Add filter too
            pSink->set_filter(
                expr::attr<SeverityLevel>("Severity").or_default(LOG_SEVERITY_INFO) >= LOG_SEVERITY_INFO // warning or greater, or
                || expr::begins_with(expr::attr<std::string>("Tag").or_default(std::string()), "IMPORTANT")); //specially tagged
        }

        Logger::Logger() {
            //Creating sinks
            enableFileSink();
            enableConsoleSink();
            //Enable attrs
            logging::add_common_attributes();
            BOOST_LOG_SCOPED_THREAD_ATTR("Uptime", attrs::timer());
            attrs::named_scope Scope;
            logging::core::get()->add_thread_attribute("Scope", Scope);
            //Indicate start
            BOOST_LOG_FUNCTION();
        }
    }
}

lc::log::Logger* lc::log::Logger::instance = nullptr;