#ifndef TORA_VALUE_BOOL_H_
#define TORA_VALUE_BOOL_H_

#include "../value.h"

namespace tora {

class BoolValue: public Value {
public:
    BoolValue(bool b): Value(VALUE_TYPE_BOOL) {
        this->bool_value_ = b;
    }
    static BoolValue* true_instance() {
        return new BoolValue(true);
    }
    static BoolValue* false_instance() {
        return new BoolValue(false);
    }
    static BoolValue* instance(bool b) {
        return b ? BoolValue::true_instance() : BoolValue::false_instance();
    }
    bool bool_value() const { return this->bool_value_; }
};

};

#endif // TORA_VALUE_BOOL_H_
