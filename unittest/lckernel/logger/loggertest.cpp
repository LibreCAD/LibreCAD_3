#include <sstream>

#include <boost/core/null_deleter.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>

#include <gtest/gtest.h>

#include <cad/logger/logger.h>

// The global logger used to be a plain src::logger_mt.  BOOST_LOG_SEV happily
// passes `severity` to such a logger as an unrecognised named parameter, which
// it then discards: every record reached the sinks without a "Severity"
// attribute.  The console sink's "warning or greater" filter therefore matched
// nothing at all -- not even LOG_ERROR -- while the unfiltered file sink got
// every LOG_TRACE line.  These tests pin the severity down at the sink, which
// is the only place the difference is observable.
namespace {
using text_sink = boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>;

/**
 * Attaches a sink carrying the same filter as the console sink and collects
 * whatever survives it.
 */
class CapturedLog {
public:
    CapturedLog()
        : _stream(boost::make_shared<std::ostringstream>()),
          _sink(boost::make_shared<text_sink>()) {
        namespace expr = boost::log::expressions;

        _sink->locked_backend()->add_stream(_stream);
        _sink->set_formatter(expr::stream
                             << "[" << expr::attr<lc::log::SeverityLevel>("Severity") << "]"
                             << expr::smessage);
        _sink->set_filter(expr::attr<lc::log::SeverityLevel>("Severity")
                              .or_default(lc::log::LOG_SEVERITY_INFO) > lc::log::LOG_SEVERITY_INFO);
        boost::log::core::get()->add_sink(_sink);
    }

    ~CapturedLog() {
        boost::log::core::get()->remove_sink(_sink);
    }

    std::string text() const {
        _sink->flush();
        return _stream->str();
    }

private:
    boost::shared_ptr<std::ostringstream> _stream;
    boost::shared_ptr<text_sink> _sink;
};
}

TEST(LoggerTest, WarningsCarryTheirSeverity) {
    CapturedLog log;

    LOG_WARNING << "kept";

    EXPECT_NE(log.text().find("[WARNING]kept"), std::string::npos)
        << "captured: " << log.text();
}

TEST(LoggerTest, ErrorsCarryTheirSeverity) {
    CapturedLog log;

    LOG_ERROR << "kept";

    EXPECT_NE(log.text().find("[ERROR]kept"), std::string::npos)
        << "captured: " << log.text();
}

// The per-entity import chatter is logged at TRACE; it must not survive a
// "warning or greater" filter, or every drawing opened floods the log again.
TEST(LoggerTest, VerboseTraceIsFilteredOut) {
    CapturedLog log;

    LOG_TRACE << "dropped";
    LOG_DEBUG << "dropped";
    LOG_INFO << "dropped";
    LOG_WARNING << "kept";

    const auto text = log.text();
    EXPECT_EQ(text.find("dropped"), std::string::npos) << "captured: " << text;
    EXPECT_NE(text.find("kept"), std::string::npos) << "captured: " << text;
}
