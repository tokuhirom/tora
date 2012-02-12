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

class FunctionFrame : public Prim {
public:
    int return_address;
    int top;
    std::vector<SharedPtr<OP>> *orig_ops;
};

class LexicalVarsFrame : public Prim {
public:
    SharedPtr<LexicalVarsFrame> up;
    std::map<int, SharedPtr<Value>> vars;
    LexicalVarsFrame() : Prim() {
        up = NULL;
    }
    LexicalVarsFrame(SharedPtr<LexicalVarsFrame> up_) : Prim() {
        this->up = up_;
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
        printf("[%d]\n", i);
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
        printf("-- dump vars(refcnt: %d) --\n", this->refcnt);
        this->dump(0);
        printf("---------------\n");
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
    std::vector<SharedPtr<FunctionFrame>> *function_frames;

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

    void dump_ops() {
        printf("-- OP DUMP    --\n");
        for (size_t i=0; i<ops->size(); i++) {
            printf("[%02zd] %s(%d)\n", i, opcode2name[ops->at(i)->op_type], ops->at(i)->op_type);
        }
        printf("----------------\n");
    }
    void dump_stack() {
        printf("-- STACK DUMP --\nSP: %d\n", sp);
        for (size_t i=0; i<stack.size(); i++) {
            printf("[%zd] ", i);
            stack.at(i)->dump();
        }
        printf("----------------\n");
    }
    void add_function(const char*name, SharedPtr<Value> code) {
        functions[std::string(name)] = code;
    }

// TODO: generate
void PP_OP_PUSH_TRUE();
void PP_OP_PUSH_FALSE();
void PP_OP_PUSH_INT();
void PP_OP_PUSH_DOUBLE();
void PP_OP_PUSH_STRING();
void PP_OP_PUSH_VALUE();
void PP_OP_NEW_RANGE();
void PP_OP_DEFINE_METHOD();
void PP_OP_SUB();
void PP_OP_DIV();
void PP_OP_MUL();
void PP_OP_ADD();
void PP_OP_FUNCALL();
void PP_OP_METHOD_CALL();
void PP_OP_RETURN();
void PP_OP_ENTER();
void PP_OP_LEAVE();
void PP_OP_PUSH_IDENTIFIER();
void PP_OP_DUMP();
void PP_OP_JUMP_IF_FALSE();
void PP_OP_JUMP();
void PP_OP_EQ();
void PP_OP_GT();
void PP_OP_LT();
void PP_OP_GE();
void PP_OP_LE();
void PP_OP_SETLOCAL();
void PP_OP_SETDYNAMIC();
void PP_OP_GETDYNAMIC();
void PP_OP_GETLOCAL();
void PP_OP_GET_ITEM();
void PP_OP_SET_ITEM();
void PP_OP_UNARY_NEGATIVE();
void PP_OP_MAKE_HASH();
void PP_OP_MAKE_ARRAY();
void PP_OP_UNARY_INCREMENT();
void PP_OP_GETGLOBAL();
void PP_OP_END();
};

};

#endif // VM_H_

