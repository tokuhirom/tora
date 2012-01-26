#ifndef VM_H_
#define VM_H_

#include "tora.h"
#include "ops.gen.h"
#include "value.h"
#include <vector>
#include <map>
#include "stack.h"

class VM {
public:
    tora::Stack stack;
    int sp; // stack pointer
    size_t pc; // program counter
    std::vector<OP*> ops;
    std::map<std::string, Value*> global_vars;

    VM() {
        sp = 0;
        pc = 0;
    }
    void execute();

    void dump_ops() {
        printf("-- OP DUMP    --\n");
        for (size_t i=0; i<ops.size(); i++) {
            printf("[%d] %s\n", i, opcode2name[ops[i]->op_type]);
        }
        printf("----------------\n");
    }
    void dump_stack() {
        printf("-- STACK DUMP --\nSP: %d\n", sp);
        for (size_t i=0; i<stack.size(); i++) {
            printf("[%d] ", i);
            stack.at(i)->dump();
        }
        printf("----------------\n");
    }
};

#endif // VM_H_

