#ifndef TORA_OP_H_
#define TORA_OP_H_

#include "value.h"

namespace tora {

struct OP {
    int op_type;
    union {
        int  int_value;
        bool bool_value;
        const char *str_value;
        struct Value *value;
    } operand;
    OP() {
    }
    OP(int type) {
        this->op_type = type;
    }
};

};

#endif // TORA_OP_H_
