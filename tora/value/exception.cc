#include "exception.h"
#include <stdarg.h>

using namespace tora;

static ExceptionImpl* exc(Value *v) {
  assert(type(v) == VALUE_TYPE_EXCEPTION);
  return static_cast<ExceptionImpl*>(v->ptr_value_);
}

ExceptionValue::ExceptionValue(const char *format, ...)
    : Value(VALUE_TYPE_EXCEPTION) {
  va_list ap;
  char p[4096 + 1];
  va_start(ap, format);
  vsnprintf(p, 4096, format, ap);
  va_end(ap);
  ptr_value_ = new ExceptionImpl(std::string(p), EXCEPTION_TYPE_GENERAL);
}

ArgumentExceptionValue::ArgumentExceptionValue(const char *format, ...) {
  va_list ap;
  char p[4096 + 1];
  va_start(ap, format);
  vsnprintf(p, 4096, format, ap);
  va_end(ap);
  ptr_value_ = new ExceptionImpl(std::string(p), EXCEPTION_TYPE_ARGUMENT);
}

exception_type_t tora::exception_type(Value* self)
{
  return exc(self)->type;
}

void tora::exception_free(Value* self)
{
  delete exc(self);
}

std::string tora::exception_message(Value* self)
{
  return exc(self)->message;
}

