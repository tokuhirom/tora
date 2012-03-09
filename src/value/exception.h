#ifndef TORA_EXCEPTION_H_
#define TORA_EXCEPTION_H_

#include "../value.h"

namespace tora {

typedef enum {
    EXCEPTION_TYPE_UNDEF,
    EXCEPTION_TYPE_STOP_ITERATION,
    EXCEPTION_TYPE_GENERAL,
    EXCEPTION_TYPE_ERRNO,
} exception_type_t;

class ExceptionValue : public Value {
    std::string message_;
    int errno_;
public:
    exception_type_t exception_type;
    ExceptionValue(const char *format, ...);
    ExceptionValue(const std::string &msg) : Value(VALUE_TYPE_EXCEPTION), errno_(0), exception_type(EXCEPTION_TYPE_GENERAL) {
        message_ = msg;
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
    const char *type_str() { return "exception"; }
};

class StopIterationExceptionValue : public ExceptionValue {
public:
    StopIterationExceptionValue() : ExceptionValue("") {
        exception_type = EXCEPTION_TYPE_STOP_ITERATION;
    }
};

};

#endif // TORA_EXCEPTION_H_