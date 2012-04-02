#ifndef TORA_EXCEPTION_H_
#define TORA_EXCEPTION_H_

#include "../exception.h"
#include "../value.h"
#include "../util.h"

namespace tora {

class ExceptionValue : public Value {
private:
protected:
    explicit ExceptionValue(exception_type_t type=EXCEPTION_TYPE_GENERAL) : Value(VALUE_TYPE_EXCEPTION) {
        exception_value_ = new ExceptionImpl(type);
    }
    explicit ExceptionValue(int err) : Value(VALUE_TYPE_EXCEPTION) {
        exception_value_ = new ExceptionImpl(err, EXCEPTION_TYPE_ERRNO);
    }
public:
    explicit ExceptionValue(const char *format, ...);
    explicit ExceptionValue(const std::string &msg, exception_type_t type=EXCEPTION_TYPE_GENERAL) : Value(VALUE_TYPE_EXCEPTION) {
        exception_value_ = new ExceptionImpl(msg, type);
    }
    virtual ~ExceptionValue() {
        delete this->exception_value_;
    }
    virtual std::string message() const {
        return this->exception_value_->message_;
    }
    exception_type_t exception_type() const {
        return this->exception_value_->type_;
    }
};

class ErrnoExceptionValue : public ExceptionValue {
public:
    ErrnoExceptionValue(int err) : ExceptionValue(err) {
    }
    int get_errno() const { return exception_value_->errno_; }
    std::string message() const {
        return tora::get_strerror(exception_value_->errno_);
    }
};

class StopIterationExceptionValue : public ExceptionValue {
public:
    StopIterationExceptionValue() : ExceptionValue(EXCEPTION_TYPE_STOP_ITERATION) {
    }
};

class ZeroDividedExceptionExceptionValue : public ExceptionValue {
public:
    ZeroDividedExceptionExceptionValue() : ExceptionValue(std::string("floating point exception: divided by zero"), EXCEPTION_TYPE_ZERO_DIVIDED) {
    }
};

class ArgumentExceptionValue : public ExceptionValue {
public:
    ArgumentExceptionValue(const char * format, ...);
};

};

#endif // TORA_EXCEPTION_H_
