#ifndef TEST_CORE_DEBUG_H
#define TEST_CORE_DEBUG_H

#include <QLoggingCategory>

// Declare logging categories
Q_DECLARE_LOGGING_CATEGORY(runtime)
Q_DECLARE_LOGGING_CATEGORY(function_parameters)

// Empty macros for testing
#define FCT_IDENTIFICATION
#define MODULE_IDENTIFICATION(x)

#endif // TEST_CORE_DEBUG_H
