#ifndef TORA_OP_H_
#define TORA_OP_H_

#include "ops.gen.h"
#include "value.h"

namespace tora {

class OP : public Value {
public:
    op_type_t op_type;
    union {
        int  int_value;
        bool bool_value;
    } operand;
    OP() {
    }
    OP(op_type_t type)  {
        this->op_type = type;
    }
    ~OP() { }
    void dump() { }
    const char *type_str() { return "OP"; };
};

/**
 * VM OP code object having Value.
 **/
class ValueOP : public OP {
public:
    Value *value;
    ValueOP(op_type_t type, Value *v) {
        this->op_type = type;
        this->value = v;
    }
    ~ValueOP();
    const char *type_str() { return "ValueOP"; };
};

};

#endif // TORA_OP_H_
