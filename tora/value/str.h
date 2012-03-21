#ifndef TORA_VALUE_STR_H_
#define TORA_VALUE_STR_H_

#include "../value.h"

namespace tora {

class StrValue: public Value {
public:
    std::string str_value;
    StrValue(): Value(VALUE_TYPE_STR) {
    }
    StrValue(const char *str): Value(VALUE_TYPE_STR) {
        this->str_value = str;
    }
    StrValue(const std::string str): Value(VALUE_TYPE_STR) {
        this->str_value = str;
    }
    ~StrValue();
    const char * c_str() {
        return this->str_value.c_str();
    }
    void set_str(const char*s) {
        str_value = s;
    }
    int length();
    void set_str(const std::string & s) {
        str_value = s;
    }
};

};

#endif // TORA_VALUE_STR_H_
