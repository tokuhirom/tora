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

#include <boost/random.hpp>

namespace tora {

class Stack;
class LexicalVarsFrame;

typedef SharedPtr<Value> (*BASIC_CALLBACK)(...);

class VM;

struct CallbackFunction {
    typedef enum {
        type_vmv = -1,
        type_vm0 = -2,
        type_vm1 = -3,
        type_vm2 = -4,
        type_vm3 = -5,
        type_vm4 = -6,
    } callback_type_t;
    typedef SharedPtr<Value> (*func_vmv_t)(VM *, const std::vector<SharedPtr<Value>>&);
    typedef SharedPtr<Value> (*func_vm0_t)(VM *);
    typedef SharedPtr<Value> (*func_vm1_t)(VM *, Value*);
    typedef SharedPtr<Value> (*func_vm2_t)(VM *, Value*, Value*);
    typedef SharedPtr<Value> (*func_vm3_t)(VM *, Value*, Value*, Value*);
    typedef SharedPtr<Value> (*func_vm4_t)(VM *, Value*, Value*, Value*, Value*);
    union {
        func_vmv_t func_vmv;
        func_vm0_t func_vm0;
        func_vm1_t func_vm1;
        func_vm2_t func_vm2;
        func_vm3_t func_vm3;
        func_vm4_t func_vm4;
    };
    int argc;
    CallbackFunction(func_vmv_t func_) : argc(type_vmv) { func_vmv = func_; }
    CallbackFunction(func_vm0_t func_) : argc(type_vm0) { func_vm0 = func_; }
    CallbackFunction(func_vm1_t func_) : argc(type_vm1) { func_vm1 = func_; }
    CallbackFunction(func_vm2_t func_) : argc(type_vm2) { func_vm2 = func_; }
    CallbackFunction(func_vm3_t func_) : argc(type_vm3) { func_vm3 = func_; }
    CallbackFunction(func_vm4_t func_) : argc(type_vm4) { func_vm4 = func_; }
};

class Package : public Value {
    ID name_id;
    std::map<ID, SharedPtr<Value>> data;
public:
    typedef std::map<ID, SharedPtr<Value>>::iterator iterator;

    Package(ID id) : Value(VALUE_TYPE_PACKAGE), name_id(id) { }
    ~Package () { }
    void add_function(ID function_name_id, SharedPtr<Value> code);

    void add_method(ID function_name_id, const CallbackFunction* code);

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
    ~PackageMap() { }
    std::map<ID, SharedPtr<Package>>::iterator find(ID id) {
        return data.find(id);
    }
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] PackageMap(%zd)\n", data.size());
    }
    void set(Package* pkg) {
        ID id = pkg->id();
        this->data[id] = pkg;
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
    ID package_id_;
    SharedPtr<Package> package_; // cached
public:
    int sp; // stack pointer
    int pc; // program counter
    SharedPtr<OPArray> ops;
    std::vector<SharedPtr<Value>> *global_vars;
    SharedPtr<SymbolTable> symbol_table;
    SharedPtr<PackageMap> package_map;
    Package* find_package(ID id);
    Package* find_package(const char *name);
    tora::Stack stack;

    std::string &package_name() {
        return symbol_table->id2name(package_id_);
    }
    void package_name(const std::string& s) {
        // will be deprecate
        package_id_ = symbol_table->get_id(s);
    }

    const SharedPtr<Package> & package() {
        return package_;
    }

    ID package_id() {
        return package_id_;
    }
    void package_id(ID id) {
        package_id_ = id;
        package_ = this->find_package(id);
    }

    std::map<ID, CallbackFunction*> builtin_functions;

    template <class T>
    void add_builtin_function(const char *name, T func) {
        ID id = this->symbol_table->get_id(std::string(name));
        this->builtin_functions.insert(std::make_pair(id, new CallbackFunction(func)));
    }

    /*
     * stack for lexical variables.
     */
    std::vector<SharedPtr<LexicalVarsFrame>> *frame_stack;
    /**
     * mark for first argument in function call.
     */
    std::vector<int> mark_stack;
    std::vector<int> stack_base;

    VM(SharedPtr<OPArray>& ops_, SharedPtr<SymbolTable> &symbol_table_);
    ~VM();
    void execute();
    void execute_trace();

    template <class operationI, class operationD> void binop(operationI operation_i, operationD operation_d);
    template <class operationI, class operationD, class operationS> SharedPtr<Value> cmpop(operationI operation_i, operationD operation_d, operationS operation_s);

    void init_globals(int argc, char**argv);

    void dump_frame();
    void dump_stack();
    SharedPtr<Value> require(Value *v);
    void add_function(ID id, SharedPtr<Value> code);
    void add_function(std::string &name, SharedPtr<Value> code) {
        this->add_function(this->symbol_table->get_id(name), code);
    }
    void die(SharedPtr<Value> & exception);
    void die(const char *format, ...);

    void register_standard_methods();

    int get_int_operand() const {
        return ops->at(pc)->operand.int_value;
    }
    double get_double_operand() const {
        return ops->at(pc)->operand.double_value;
    }

    SharedPtr<Value> copy_all_public_symbols(ID srcid, ID dstid);

    void call_native_func(const CallbackFunction* callback, int argcnt);
    void add(SharedPtr<Value>& v1, const SharedPtr<Value>& v2);

    Value* TOP() { return stack.top(); }
    SharedPtr<Value> unary_negative(const SharedPtr<Value>& v);
    SharedPtr<Value> set_item(const SharedPtr<Value>& container, const SharedPtr<Value>& index, const SharedPtr<Value>& rvalue) const;

    boost::mt19937 *myrand;

#include "vm.ops.inc.h"
};

class VM;

};

#endif // VM_H_

