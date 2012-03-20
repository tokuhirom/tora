#ifndef TORA_EXCEPTION_H_
#define TORA_EXCEPTION_H_

#include "../value.h"

namespace tora {

typedef enum {
    EXCEPTION_TYPE_UNDEF,
    EXCEPTION_TYPE_STOP_ITERATION,
    EXCEPTION_TYPE_ZERO_DIVIDED,
    EXCEPTION_TYPE_GENERAL,
    EXCEPTION_TYPE_ARGUMENT,
    EXCEPTION_TYPE_ERRNO,
} exception_type_t;

class ExceptionValue : public Value {
protected:
    std::string message_;
    int errno_;
protected:
    ExceptionValue(exception_type_t type=EXCEPTION_TYPE_GENERAL) : Value(VALUE_TYPE_EXCEPTION), errno_(0), exception_type(type) {
    }
public:
    exception_type_t exception_type;
    ExceptionValue(const char *format, ...);
    ExceptionValue(const std::string &msg, exception_type_t type=EXCEPTION_TYPE_GENERAL) : Value(VALUE_TYPE_EXCEPTION), message_(msg), errno_(0), exception_type(type) {
    }
    ExceptionValue(int err) : Value(VALUE_TYPE_EXCEPTION), errno_(err), exception_type(EXCEPTION_TYPE_ERRNO) { }
    int get_errno() { return this->errno_; }
    std::string message() {
        if (errno_) {
            return std::string(strerror(this->errno_));
        } else {
            return message_;
        }
    }
};

class StopIterationExceptionValue : public ExceptionValue {
public:
    StopIterationExceptionValue() : ExceptionValue("") {
        exception_type = EXCEPTION_TYPE_STOP_ITERATION;
    }
};

class ZeroDividedExceptionExceptionValue : public ExceptionValue {
public:
    ZeroDividedExceptionExceptionValue() : ExceptionValue("floating point exception: divided by zero") {
        exception_type = EXCEPTION_TYPE_ZERO_DIVIDED;
    }
};

class ArgumentExceptionValue : public ExceptionValue {
public:
    ArgumentExceptionValue(const char * format, ...);
};

};

#endif // TORA_EXCEPTION_H_
