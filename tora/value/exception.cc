#include "exception.h"
#include <stdarg.h>

using namespace tora;

ExceptionValue::ExceptionValue(const char *format, ...)
    : Value(VALUE_TYPE_EXCEPTION) {
  va_list ap;
  char p[4096 + 1];
  va_start(ap, format);
  vsnprintf(p, 4096, format, ap);
  va_end(ap);
  exception_value_ = new ExceptionImpl(std::string(p), EXCEPTION_TYPE_GENERAL);
}

ArgumentExceptionValue::ArgumentExceptionValue(const char *format, ...) {
  va_list ap;
  char p[4096 + 1];
  va_start(ap, format);
  vsnprintf(p, 4096, format, ap);
  va_end(ap);
  exception_value_ = new ExceptionImpl(std::string(p), EXCEPTION_TYPE_ARGUMENT);
}
