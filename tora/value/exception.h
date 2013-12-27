#ifndef TORA_EXCEPTION_H_
#define TORA_EXCEPTION_H_

#include "../exception.h"
#include "../value.h"
#include "../util.h"

namespace tora {

struct ExceptionImpl {
  friend class ExceptionValue;
  friend class ErrnoExceptionValue;
  friend class ArgumentExceptionValue;

 public:
  exception_type_t type;
  int errno_;
  std::string message;
  explicit ExceptionImpl(exception_type_t _type) : type(_type) {}
  explicit ExceptionImpl(int err, exception_type_t _type = EXCEPTION_TYPE_ERRNO)
      : type(_type), errno_(err) {}
  explicit ExceptionImpl(const std::string& msg, exception_type_t _type)
      : type(_type), message(msg) {}
};

  std::string exception_message(Value* self);
  void exception_free(Value* self);
  exception_type_t exception_type(Value* self);

class ExceptionValue : public Value {
 private:
 protected:
  explicit ExceptionValue(exception_type_t type = EXCEPTION_TYPE_GENERAL)
      : Value(VALUE_TYPE_EXCEPTION) {
    ptr_value_ = new ExceptionImpl(type);
  }
  explicit ExceptionValue(int err) : Value(VALUE_TYPE_EXCEPTION) {
    ptr_value_ = new ExceptionImpl(err, EXCEPTION_TYPE_ERRNO);
  }

 public:
  explicit ExceptionValue(const char *format, ...);
  explicit ExceptionValue(const std::string &msg,
                          exception_type_t type = EXCEPTION_TYPE_GENERAL)
      : Value(VALUE_TYPE_EXCEPTION) {
    ptr_value_ = new ExceptionImpl(msg, type);
  }
};

class ErrnoExceptionValue : public ExceptionValue {
 public:
  ErrnoExceptionValue(int err) : ExceptionValue(err) {}
  int get_errno() const { return static_cast<ExceptionImpl*>(ptr_value_)->errno_; }
  std::string message() const {
    return tora::get_strerror(static_cast<ExceptionImpl*>(ptr_value_)->errno_);
  }
};

class StopIterationExceptionValue : public ExceptionValue {
 public:
  StopIterationExceptionValue()
      : ExceptionValue(EXCEPTION_TYPE_STOP_ITERATION) {}
};

class ZeroDividedExceptionExceptionValue : public ExceptionValue {
 public:
  ZeroDividedExceptionExceptionValue()
      : ExceptionValue(std::string("floating point exception: divided by zero"),
                       EXCEPTION_TYPE_ZERO_DIVIDED) {}
};

class ArgumentExceptionValue : public ExceptionValue {
 public:
  ArgumentExceptionValue(const char *format, ...);
};
};

#endif  // TORA_EXCEPTION_H_
