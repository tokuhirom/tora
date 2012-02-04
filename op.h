#ifndef TORA_OP_H_
#define TORA_OP_H_

#include "ops.gen.h"

namespace tora {

class Value;

class OP {
public:
    op_type_t op_type;
    union {
        int  int_value;
        bool bool_value;
    } operand;
    OP() {
    }
    OP(op_type_t type) {
        this->op_type = type;
    }
    ~OP() { }
};

class ValueOP : public OP {
public:
    Value *value;
    ValueOP(op_type_t type, Value *v) {
        this->op_type = type;
        this->value = v;
    }
    ~ValueOP();
};

};

#endif // TORA_OP_H_
