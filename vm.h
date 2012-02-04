#ifndef VM_H_
#define VM_H_

#include "tora.h"
#include "ops.gen.h"
#include "value.h"
#include <vector>
#include <map>
#include "stack.h"

namespace tora {

class Stack;

class FunctionFrame : public Value {
public:
    int return_address;
    int top;
    std::vector<OP*> *orig_ops;
    void dump() {
        printf("FunctionFrame\n");
    }
    const char *type_str() { return "FunctionFrame"; }
};

class LexicalVarsFrame : public Value {
public:
    LexicalVarsFrame* up;
    std::map<int, Value*> vars;
    LexicalVarsFrame() {
        up = NULL;
    }
    LexicalVarsFrame(LexicalVarsFrame *up_) {
        up_->retain();
        this->up = up_;
    }
    void setVar(int id, Value *v) {
        this->vars[id] = v;
    }
    Value *find(int id) {
        std::map<int, Value*>::iterator iter = this->vars.find(id);
        if (iter != vars.end()) {
            return iter->second;
        } else {
            if (this->up) {
                return this->up->find(id);
            }
            return NULL;
        }
    }
    void dump(int i) {
        printf("[%d]\n", i);
        std::map<int, Value*>::iterator iter = this->vars.begin();
        for (; iter!=this->vars.end(); ++iter) {
            printf("  %d\n", iter->first);
            iter->second->dump();
        }
        if (this->up) {
            this->up->dump(i+1);
        }
    }
    void dump() {
        printf("-- dump vars(refcnt: %d) --\n", this->refcnt);
        this->dump(0);
        printf("---------------\n");
    }
    const char *type_str() { return "LexicalVarsFrame"; }
};

class VM {
public:
    tora::Stack stack;
    int sp; // stack pointer
    size_t pc; // program counter
    std::vector<OP*> *ops;
    std::map<std::string, Value*> global_vars;
    std::map<std::string, Value*> functions;
    std::vector<FunctionFrame*> *function_frames;

    /*
     * stack for lexical variables.
     */
    std::vector<LexicalVarsFrame *> *lexical_vars_stack;

    VM();
    ~VM();
    void execute();

    void dump_ops() {
        printf("-- OP DUMP    --\n");
        for (size_t i=0; i<ops->size(); i++) {
            printf("[%02d] %s(%d)\n", i, opcode2name[ops->at(i)->op_type], ops->at(i)->op_type);
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

