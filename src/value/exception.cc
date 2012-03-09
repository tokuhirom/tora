#include "exception.h"
#include <stdarg.h>

using namespace tora;

ExceptionValue::ExceptionValue(const char *format, ...)
    : Value(VALUE_TYPE_EXCEPTION), errno_(0), exception_type(EXCEPTION_TYPE_GENERAL) {

    va_list ap;
    char p[4096+1];
    va_start(ap, format);
    vsnprintf(p, 4096, format, ap);
    va_end(ap);
    message_ = p;
}
