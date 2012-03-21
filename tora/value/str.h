#ifndef TORA_VALUE_STR_H_
#define TORA_VALUE_STR_H_

#include "../value.h"

namespace tora {

class StrValue: public Value {
public:
    StrValue(): Value(VALUE_TYPE_STR) {
        value_ = "";
    }
    StrValue(const char *str): Value(VALUE_TYPE_STR) {
        value_ = std::string(str);
    }
    StrValue(const std::string str): Value(VALUE_TYPE_STR) {
        value_ = str;
    }
    ~StrValue() { }
    const char * c_str() {
        return boost::get<std::string>(this->value_).c_str();
    }
    void set_str(const char*s) {
        value_ = s;
    }
    int length();
    void set_str(const std::string & s) {
        value_ = s;
    }
    const std::string & str_value() const {
        return boost::get<std::string>(value_);
    }
    std::string & str_value() {
        return boost::get<std::string>(value_);
    }
};

};

#endif // TORA_VALUE_STR_H_
