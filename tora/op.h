#ifndef TORA_OP_H_
#define TORA_OP_H_

#include "ops.gen.h"
#include "shared_ptr.h"
#include "prim.h"

namespace tora {

class Value;

class OP {
    PRIM_DECL();
public:
    op_type_t op_type;
    union {
        int  int_value;
        double double_value;
        bool bool_value;
    } operand;
    OP() :refcnt(0) { }
    OP(op_type_t type) :refcnt(0), op_type(type)  { }
    OP(op_type_t type, int i) :refcnt(0), op_type(type)  {
        operand.int_value = i;
    }
    OP(op_type_t type, int high, int low) :refcnt(0), op_type(type)  {
        operand.int_value = (((high)&0x0000ffff) << 16) | (low&0x0000ffff);
    }
    virtual ~OP() { }

    int int_operand_high() const {
        return (operand.int_value >> 16) & 0x0000FFFF;
    }
    int int_operand_low() const {
        return operand.int_value & 0x0000ffff;
    }

    template<class Y>
    Y* upcast() {
        return static_cast<Y*>(&(*(this)));
    }
};

/**
 * VM OP code object having Value.
 **/
class ValueOP : public OP {
public:
    SharedPtr<Value> value;
    ValueOP(op_type_t type, const SharedPtr<Value>& v)
        : OP(type)
        , value(v) {
    }
    ValueOP(op_type_t type, Value* v)
        : OP(type)
        , value(v) {
    }
};

};

#endif // TORA_OP_H_
