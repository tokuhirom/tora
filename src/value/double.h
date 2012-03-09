#ifndef TORA_VALUE_DOUBLE_H_
#define TORA_VALUE_DOUBLE_H_

#include "../value.h"

namespace tora {

class DoubleValue: public Value {
public:
    double  double_value;
    DoubleValue(double d): Value(VALUE_TYPE_DOUBLE) {
        this->double_value = d;
    }
    const char *type_str() { return "double"; }
};

};

#endif // TORA_VALUE_DOUBLE_H_
