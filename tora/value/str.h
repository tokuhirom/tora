#ifndef TORA_VALUE_STR_H_
#define TORA_VALUE_STR_H_

#include "../value.h"

namespace tora {

class StrValue: public Value {
private:
    const std::string & VAL() const {
        return **(this->str_value_);
    }
public:
    StrValue(): Value(VALUE_TYPE_STR) {
        str_value_ = new boost::shared_ptr<std::string>(new std::string(""));
    }
    StrValue(const char *str): Value(VALUE_TYPE_STR) {
        str_value_ = new boost::shared_ptr<std::string>(new std::string(str));
    }
    StrValue(const char *str, size_t t): Value(VALUE_TYPE_STR) {
        str_value_ = new boost::shared_ptr<std::string>(new std::string(str, t));
    }
    StrValue(const std::string &str): Value(VALUE_TYPE_STR) {
        str_value_ = new boost::shared_ptr<std::string>(new std::string(str));
    }
    ~StrValue() { delete str_value_; }
    const char * c_str() {
        return this->str_value_->get()->c_str();
    }
    int length();
    void set_str(const std::string & s) {
        **str_value_ = s;
    }
    std::string str_value() const {
        return **str_value_;
    }
};

};

#endif // TORA_VALUE_STR_H_
