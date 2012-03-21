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
private:
protected:
    ExceptionValue(exception_type_t type=EXCEPTION_TYPE_GENERAL) : Value(VALUE_TYPE_EXCEPTION), exception_type(type) {
    }
public:
    exception_type_t exception_type;
    ExceptionValue(const char *format, ...);
    ExceptionValue(const std::string &msg, exception_type_t type=EXCEPTION_TYPE_GENERAL) : Value(VALUE_TYPE_EXCEPTION), exception_type(type) {
        value_ = msg;
    }
    virtual std::string message() {
        return boost::get<std::string>(value_);
    }
};

class ErrnoExceptionValue : public ExceptionValue {
public:
    ErrnoExceptionValue(int err) : ExceptionValue(EXCEPTION_TYPE_ERRNO){
        value_ = err;
    }
    int get_errno() const { return boost::get<int>(value_); }
    std::string message() const {
        return std::string(strerror(get_errno()));
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
