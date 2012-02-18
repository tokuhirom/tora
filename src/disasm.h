#ifndef TORA_DISASM_H_
#define TORA_DISASM_H_

#include "op.h"

namespace tora {

class Disasm {
public:
    static void disasm_op(OP* op);

    static void disasm(std::vector<SharedPtr<OP>> *ops);
};

};

#endif // TORA_DISASM_H_
