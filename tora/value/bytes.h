#ifndef TORA_VALUE_BYTES_H_
#define TORA_VALUE_BYTES_H_

#include "../value.h"

namespace tora {

class BytesValue: public Value {
private:
    const std::string & VAL() const {
        return *(this->str_value_);
    }
public:
    BytesValue(): Value(VALUE_TYPE_BYTES) {
    }
    BytesValue(const char *str): Value(VALUE_TYPE_BYTES) {
        this->str_value_ = new std::string(str);
    }
    BytesValue(const char *str, size_t len): Value(VALUE_TYPE_BYTES) {
        this->str_value_ = new std::string(str, len);
    }
    BytesValue(const std::string &str): Value(VALUE_TYPE_BYTES) {
        this->str_value_ = new std::string(str);
    }
    ~BytesValue() { delete str_value_; }
    std::string str_value() const {
        return VAL();
    }
    const char * c_str() const {
        return VAL().c_str();
    }
    void set_str(const char*s) {
        *str_value_ = s;
    }
    int length();
    size_t size() {
        return VAL().size();
    }
    void set_str(const std::string & s) {
        *str_value_ = s;
    }
};

};

#endif // TORA_VALUE_BYTES_H_
