#ifndef VM_H_
#define VM_H_

#include "tora.h"
#include "op.h"
#include "value.h"
#include <vector>
#include <map>
#include "stack.h"
#include "shared_ptr.h"

namespace tora {

class Stack;

typedef enum {
    FRAME_TYPE_LEXICAL = 1,
    FRAME_TYPE_FUNCTION,
    FRAME_TYPE_TRY,
} frame_type_t;

// TODO rename LexicalVarsFrame to Frame
class LexicalVarsFrame : public Prim {
public:
    frame_type_t type;
    int top;
    SharedPtr<LexicalVarsFrame> up;
    std::map<int, SharedPtr<Value>> vars;
    LexicalVarsFrame() : Prim() {
        up = NULL;
        type = FRAME_TYPE_LEXICAL;
    }
    LexicalVarsFrame(SharedPtr<LexicalVarsFrame> up_) : Prim() {
        this->up = up_;
        type = FRAME_TYPE_LEXICAL;
    }
    ~LexicalVarsFrame() { }
    void setVar(int id, SharedPtr<Value> v) {
        this->vars[id] = v;
    }
    Value *find(int id) {
        auto iter = this->vars.find(id);
        if (iter != vars.end()) {
            return &(*(iter->second));
        } else {
            if (this->up) {
                return this->up->find(id);
            }
            return NULL;
        }
    }
    void dump(int i) {
        printf("type: %s [%d]\n", this->type == FRAME_TYPE_FUNCTION ? "function" : "lexical", i);
        auto iter = this->vars.begin();
        for (; iter!=this->vars.end(); ++iter) {
            printf("  %d\n", iter->first);
            iter->second->dump();
        }
        if (this->up) {
            this->up->dump(i+1);
        }
    }
    void dump() {
        printf("-- dump vars (refcnt: %d) --\n", this->refcnt);
        this->dump(0);
        printf("---------------\n");
    }

    template<class Y>
    Y* upcast() {
        return dynamic_cast<Y*>(&(*(this)));
    }

};

class FunctionFrame : public LexicalVarsFrame {
public:
    int return_address;
    std::vector<SharedPtr<OP>> *orig_ops;
    FunctionFrame(SharedPtr<LexicalVarsFrame> up_) : LexicalVarsFrame(up_) {
        this->type = FRAME_TYPE_FUNCTION;
    }
};


class VM {
public:
    tora::Stack stack;
    int sp; // stack pointer
    size_t pc; // program counter
    std::vector<SharedPtr<OP>> *ops;
    std::vector<SharedPtr<Value>> *global_vars;
    std::map<std::string, SharedPtr<Value>> functions;

    /*
     * stack for lexical variables.
     */
    std::vector<SharedPtr<LexicalVarsFrame>> *lexical_vars_stack;

    VM(std::vector<SharedPtr<OP>>* ops_);
    ~VM();
    void execute();

    template <class operationI, class operationD> void binop(operationI operation_i, operationD operation_d);
    template <class operationI, class operationD> void cmpop(operationI operation_i, operationD operation_d);

    void init_globals(int argc, char**argv);

    void dump_stack() {
        printf("-- STACK DUMP --\nSP: %d\n", sp);
        for (size_t i=0; i<stack.size(); i++) {
            printf("[%zd] ", i);
            stack.at(i)->dump();
        }
        printf("----------------\n");
    }
    void add_function(std::string &name, SharedPtr<Value> code) {
        functions[name] = code;
    }

#include "vm.ops.inc.h"
};

};

#endif // VM_H_

