#ifndef VM_H_
#define VM_H_

#include "tora.h"
#include "ops.gen.h"
#include "value.h"
#include <vector>
#include <map>
#include "stack.h"

namespace tora {

class FunctionFrame {
public:
    int return_address;
    int top;
};

class LexicalVarsFrame {
    LexicalVarsFrame* up;
public:
    std::map<std::string, Value*> vars;
    LexicalVarsFrame() {
        up = NULL;
    }
    LexicalVarsFrame(LexicalVarsFrame *up_) {
        this->up = up_;
    }
    void setVar(std::string* name, Value *v) {
        this->vars[*name] = v;
    }
    Value *find(std::string name) {
        std::map<std::string, Value*>::iterator iter = this->vars.find(std::string(name));
        if (iter != vars.end()) {
            return iter->second;
        } else {
            if (this->up) {
                return this->up->find(name);
            }
            return NULL;
        }
    }
};

class VM {
public:
    tora::Stack stack;
    int sp; // stack pointer
    size_t pc; // program counter
    std::vector<OP*> ops;
    std::map<std::string, Value*> global_vars;
    std::map<std::string, Value*> functions;
    std::vector<FunctionFrame*> *function_frames;

    /*
     * stack for lexical variables.
     */
    std::vector<LexicalVarsFrame *> *lexical_vars_stack;

    VM() {
        sp = 0;
        pc = 0;
    }
    void execute();

    void dump_ops() {
        printf("-- OP DUMP    --\n");
        for (size_t i=0; i<ops.size(); i++) {
            printf("[%d] %s(%d)\n", i, opcode2name[ops[i]->op_type], ops[i]->op_type);
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
    void add_function(const char*name, Value*code) {
        functions[std::string(name)] = code;
    }
};

};

#endif // VM_H_

