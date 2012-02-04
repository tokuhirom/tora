#ifndef TORA_OP_H_
#define TORA_OP_H_

namespace tora {

class Value;

class OP {
public:
    int op_type;
    union {
        int  int_value;
        bool bool_value;
    } operand;
    OP() {
    }
    OP(int type) {
        this->op_type = type;
    }
    ~OP() { }
};

class ValueOP : public OP {
public:
    Value *value;
    ValueOP(int type, Value *v) {
        this->op_type = type;
        this->value = v;
    }
    ~ValueOP();
};

};

#endif // TORA_OP_H_
