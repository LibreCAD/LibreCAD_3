First see example

https:://github.com/boostorg/log/blob/develop/example/advanced_usage/main.cpp

The boost::log is quite diffrent then expected so.

It is in class so it can be updated or any feature can be added.(later like storing it in files or getting log over network)

Actually here in class there only init function that creates sinks. So to write log first create logger source and start writing log.

#include "cad/logger/logger.h"
	lc::log::lc_logger slg;// it is simply boost::log::sources::severity_logger<severity_level>
	BOOST_LOG(slg) << "Hello World!";//No changes in macro	or, LOG
	BOOST_LOG_SEV(slg,error) << "Error msg..";//minimal	or, LOGS

Available severirities:
    normal
    notification <--- can be used to show notification if needed .. later
    warning
    error
    critical

All the records are written in file but console provides the minimal.
Only important tags are shown in console or severity >= warning.

The things stored in file are:-
- Time
- Severity
- Tags
- Scope

Use of tags and Scope depends on source, i mean slg in example.
