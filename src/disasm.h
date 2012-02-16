#ifndef TORA_DISASM_H_
#define TORA_DISASM_H_

#include "op.h"

namespace tora {

class Disasm {
public:
    static void disasm_op(OP* op);

    static void disasm(std::vector<SharedPtr<OP>> *ops) {
        printf("-- OP DUMP    --\n");
        for (size_t i=0; i<ops->size(); i++) {
            printf("[%02zd] %s(%d)\n", i, opcode2name[ops->at(i)->op_type], ops->at(i)->op_type);
        }
        printf("----------------\n");
    }
};

};

#endif // TORA_DISASM_H_
