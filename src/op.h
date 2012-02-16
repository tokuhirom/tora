#ifndef TORA_OP_H_
#define TORA_OP_H_

#include "ops.gen.h"
#include "value.h"
#include "shared_ptr.h"

namespace tora {

class OP : public Prim {
public:
    op_type_t op_type;
    union {
        int  int_value;
        double double_value;
        bool bool_value;
    } operand;
    OP() { }
    OP(op_type_t type)  {
        this->op_type = type;
    }
    virtual ~OP() { }
};

/**
 * VM OP code object having Value.
 **/
class ValueOP : public OP {
public:
    SharedPtr<Value> value;
    ValueOP(op_type_t type, SharedPtr<Value> v) {
        this->op_type = type;
        this->value = v;
    }
    ~ValueOP() { }
};

};

#endif // TORA_OP_H_