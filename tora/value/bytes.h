#ifndef TORA_VALUE_BYTES_H_
#define TORA_VALUE_BYTES_H_

#include "../value.h"

namespace tora {

class BytesValue: public Value {
private:
    const std::string & VAL() const {
        return boost::get<std::string>(this->value_);
    }
public:
    BytesValue(): Value(VALUE_TYPE_BYTES) {
    }
    BytesValue(const char *str): Value(VALUE_TYPE_BYTES) {
        this->value_ = std::string(str);
    }
    BytesValue(const std::string str): Value(VALUE_TYPE_BYTES) {
        this->value_ = str;
    }
    ~BytesValue() { }
    const char * c_str() {
        return VAL().c_str();
    }
    void set_str(const char*s) {
        value_ = s;
    }
    int length();
    void set_str(const std::string & s) {
        value_ = s;
    }
};

};

#endif // TORA_VALUE_BYTES_H_
