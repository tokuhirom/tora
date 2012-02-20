#ifndef VM_H_
#define VM_H_

#include "tora.h"
#include "op.h"
#include "value.h"
#include <vector>
#include <map>
#include "stack.h"
#include "shared_ptr.h"
#include "symbol_table.h"

namespace tora {

class Stack;

typedef enum {
    FRAME_TYPE_LEXICAL = 1,
    FRAME_TYPE_FUNCTION,
    FRAME_TYPE_TRY,
} frame_type_t;

// TODO rename LexicalVarsFrame to Frame
class LexicalVarsFrame : public Prim {
private:
    std::map<int, SharedPtr<Value>> vars;
public:
    frame_type_t type;
    int top;
    SharedPtr<LexicalVarsFrame> up;
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

class TryFrame : public LexicalVarsFrame {
public:
    int return_address;
    TryFrame(SharedPtr<LexicalVarsFrame> up_) : LexicalVarsFrame(up_) {
        this->type = FRAME_TYPE_TRY;
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

typedef SharedPtr<Value> (*BASIC_CALLBACK)(...);

class VM;

struct Callback {
    typedef SharedPtr<Value> (*func0_t)(SharedPtr<Value>&);
    typedef SharedPtr<Value> (*func1_t)(SharedPtr<Value>&, SharedPtr<Value>&);
    union {
        func0_t func0;
        func1_t func1;
    };
    int argc;
    Callback(func0_t func_) : argc(0) {
        func0 = func_;
    }
    Callback(func1_t func_) : argc(1) {
        func1 = func_;
    }
};

struct CallbackFunction {
    typedef SharedPtr<Value> (*func0_t)();
    typedef SharedPtr<Value> (*func1_t)(SharedPtr<Value>&);
    typedef SharedPtr<Value> (*funcv_t)(const std::vector<SharedPtr<Value>>&);
    union {
        func0_t func0;
        func1_t func1;
        funcv_t funcv;
    };
    int argc;
    CallbackFunction(func0_t func_) : argc(0) {
        func0 = func_;
    }
    CallbackFunction(func1_t func_) : argc(1) {
        func1 = func_;
    }
    CallbackFunction(funcv_t func_) : argc(-1) {
        funcv = func_;
    }
};

class VM {
public:
    tora::Stack stack;
    int sp; // stack pointer
    size_t pc; // program counter
    std::vector<SharedPtr<OP>> *ops;
    std::vector<SharedPtr<Value>> *global_vars;
    std::map<ID, SharedPtr<Value>> functions;
    SharedPtr<SymbolTable> symbol_table;

    std::map<ID, CallbackFunction*> builtin_functions;
    template <class T>
    void add_builtin_function(const char *name, T func) {
        ID id = this->symbol_table->get_id(std::string(name));
        this->builtin_functions.insert(std::make_pair(id, new CallbackFunction(func)));
    }

    std::map<value_type_t, std::map<ID, Callback*>*> standard;

    /*
     * stack for lexical variables.
     */
    std::vector<SharedPtr<LexicalVarsFrame>> *frame_stack;

    VM(std::vector<SharedPtr<OP>>* ops_, SharedPtr<SymbolTable> &symbol_table_);
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
    void add_function(ID id, SharedPtr<Value> code) {
        functions[id] = code;
    }
    void add_function(std::string &name, SharedPtr<Value> code) {
        this->add_function(this->symbol_table->get_id(name), code);
    }
    void die(SharedPtr<Value> & exception);

    void register_standard_methods();

#include "vm.ops.inc.h"
};

class MetaClass {
    VM *vm_;
    value_type_t type;
    std::map<ID, Callback*> *methods;
public:
    MetaClass(VM *v, value_type_t t) : vm_(v), type(t) {
        this->methods = new std::map<ID, Callback*>();
        vm_->standard[t] = this->methods;
    }
    template <class T>
    void add_method(const std::string &name, T func) {
        ID id = vm_->symbol_table->get_id(name);
        this->methods->insert(std::make_pair(id, new Callback(func)));
    }
};


};

#endif // VM_H_

