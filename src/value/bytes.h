#ifndef TORA_VALUE_BYTES_H_
#define TORA_VALUE_BYTES_H_

#include "../value.h"

namespace tora {

class BytesValue: public Value {
public:
    std::string bytes_value;
    BytesValue(): Value(VALUE_TYPE_BYTES) {
    }
    BytesValue(const char *str): Value(VALUE_TYPE_BYTES) {
        this->bytes_value = str;
    }
    BytesValue(const std::string str): Value(VALUE_TYPE_BYTES) {
        this->bytes_value = str;
    }
    ~BytesValue();
    const char * c_str() {
        return this->bytes_value.c_str();
    }
    void set_str(const char*s) {
        bytes_value = s;
    }
    int length();
    void set_str(const std::string & s) {
        bytes_value = s;
    }
};

};

#endif // TORA_VALUE_BYTES_H_
