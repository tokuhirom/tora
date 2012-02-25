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
#include "op_array.h"
#include "value/hash.h"
#include "value/code.h"
#include <stdarg.h>

namespace tora {

class Stack;

typedef enum {
    FRAME_TYPE_LEXICAL = 1,
    FRAME_TYPE_FUNCTION,
    FRAME_TYPE_TRY,
    FRAME_TYPE_FOREACH,
    FRAME_TYPE_PACKAGE,
} frame_type_t;

// TODO rename LexicalVarsFrame to Frame
class LexicalVarsFrame : public Prim {
private:
    std::vector<SharedPtr<Value>> *vars;
public:
    frame_type_t type;
    int top;
    SharedPtr<LexicalVarsFrame> up;
    SharedPtr<CodeValue> code;
    LexicalVarsFrame(int vars_cnt) : Prim() {
        up = NULL;
        type = FRAME_TYPE_LEXICAL;
        vars = new std::vector<SharedPtr<Value>>(vars_cnt);
    }
    LexicalVarsFrame(int vars_cnt, SharedPtr<LexicalVarsFrame> up_) : Prim() {
        this->up = up_;
        type = FRAME_TYPE_LEXICAL;
        vars = new std::vector<SharedPtr<Value>>(vars_cnt);
    }
    ~LexicalVarsFrame() {
        delete vars;
    }
    void setVar(int id, SharedPtr<Value> v) {
        assert(id < this->vars->capacity());
        (*this->vars)[id] = v;
    }
    SharedPtr<Value> find(int id) {
        assert(id < this->vars->capacity());
        return (*this->vars)[id];
    }
    void dump(int i) {
        printf("type: %s [%d]\n", this->type == FRAME_TYPE_FUNCTION ? "function" : "lexical", i);
        for (size_t n=0; n<this->vars->size(); n++) {
            printf("  %zd\n", n);
            this->vars->at(n)->dump();
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
    // try frame does not have variables.
    TryFrame(SharedPtr<LexicalVarsFrame> up_) : LexicalVarsFrame(0, up_) {
        this->type = FRAME_TYPE_TRY;
    }
};

class FunctionFrame : public LexicalVarsFrame {
public:
    int return_address;
    SharedPtr<OPArray> orig_ops;
    SharedPtr<Value> self;
    FunctionFrame(int vars_cnt, SharedPtr<LexicalVarsFrame> up_) : LexicalVarsFrame(vars_cnt, up_) {
        this->type = FRAME_TYPE_FUNCTION;
    }
    FunctionFrame(SharedPtr<Value>& self_, int vars_cnt, SharedPtr<LexicalVarsFrame> up_) : LexicalVarsFrame(vars_cnt, up_) {
        self = self_;
    }
};

class ForeachFrame : public LexicalVarsFrame {
public:
    SharedPtr<Value> iter;
    ForeachFrame(int vars_cnt, SharedPtr<LexicalVarsFrame> up_) : LexicalVarsFrame(vars_cnt, up_) {
        this->type = FRAME_TYPE_FOREACH;
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
    typedef SharedPtr<Value> (*func_vm0_t)(VM * vm_);
    typedef SharedPtr<Value> (*func_vm1_t)(VM * vm_, SharedPtr<Value>&);
    union {
        func0_t    func0;
        func1_t    func1;
        funcv_t    funcv;
        func_vm0_t func_vm0;
        func_vm1_t func_vm1;
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
    CallbackFunction(func_vm0_t func_) : argc(-2) { func_vm0 = func_; }
    CallbackFunction(func_vm1_t func_) : argc(-3) { func_vm1 = func_; }
};

class Package : public Value {
    ID name_id;
    std::map<ID, SharedPtr<Value>> data;
public:
    typedef std::map<ID, SharedPtr<Value>>::iterator iterator;

    Package(ID id) : Value(VALUE_TYPE_PACKAGE), name_id(id) { }
    void add_function(ID function_name_id, SharedPtr<Value> code);
    iterator find(ID id) {
        return data.find(id);
    }
    virtual void dump(SharedPtr<SymbolTable> & symbol_table, int indent) {
        print_indent(indent);
        printf("[dump] Package(%s):\n", symbol_table->id2name(name_id).c_str());
        auto iter = data.begin();
        for (; iter!=data.end(); iter++) {
            print_indent(indent+1);
            printf("%s:\n", symbol_table->id2name(iter->first).c_str());
            iter->second->dump(indent+2);
        }
    }
    virtual void dump(int indent) {
        print_indent(indent);
        printf("[dump] Package\n");
    }
    const char *type_str() { return "package"; }
    ID id() { return name_id; }
    iterator begin() { return data.begin(); }
    iterator end()   { return data.end(); }
};

class PackageMap : public Value {
    std::map<ID, SharedPtr<Package>> data;
public:
    PackageMap() : Value(VALUE_TYPE_PACKAGE_MAP) {
    }
    std::map<ID, SharedPtr<Package>>::iterator find(ID id) {
        return data.find(id);
    }
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] PackageMap(%zd)\n", data.size());
    }
    void set(SharedPtr<Package> &pkg) {
        this->data[pkg->id()] = pkg;
    }
    const char *type_str() { return "package_map"; }
    std::map<ID, SharedPtr<Package>>::iterator end() {
        return data.end();
    }
};

class VM {
public:
    tora::Stack stack;
    int sp; // stack pointer
    int pc; // program counter
    SharedPtr<OPArray> ops;
    std::vector<SharedPtr<Value>> *global_vars;
    SharedPtr<SymbolTable> symbol_table;
    std::string package;
    SharedPtr<PackageMap> package_map;
    SharedPtr<Package> find_package(ID id);

    // TODO: cache
    ID package_id() {
        return this->symbol_table->get_id(package);
    }

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

    VM(SharedPtr<OPArray>& ops_, SharedPtr<SymbolTable> &symbol_table_);
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
    SharedPtr<Value> require(SharedPtr<Value> &v);
    void add_function(ID id, SharedPtr<Value> code);
    void add_function(std::string &name, SharedPtr<Value> code) {
        this->add_function(this->symbol_table->get_id(name), code);
    }
    void die(SharedPtr<Value> & exception);
    void die(const char *format, ...);

    void register_standard_methods();

    int get_int_operand() {
        return ops->at(pc)->operand.int_value;
    }
    int get_double_operand() {
        return ops->at(pc)->operand.double_value;
    }

    SharedPtr<Value> copy_all_public_symbols(ID srcid, const std::string &dst);

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

class VM;

class PackageFrame : public LexicalVarsFrame {
    std::string orig_package;
    VM * vm_;
public:
    PackageFrame(std::string pkg, VM *parent, SharedPtr<LexicalVarsFrame> up_) : LexicalVarsFrame(0, up_) {
        this->type = FRAME_TYPE_PACKAGE;
        orig_package = pkg;
        vm_ = parent;
    }
    ~PackageFrame() {
        // printf("LEAVE PACKAGE FROM %s. back to %s\n", vm_->package.c_str(), orig_package.c_str());
        vm_->package = orig_package;
    }
};


};

#endif // VM_H_

