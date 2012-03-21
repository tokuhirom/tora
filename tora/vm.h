#ifndef VM_H_
#define VM_H_

#include <vector>
#include <map>
#include <set>
#include <stdarg.h>

#include "tora.h"
#include "op.h"
#include "value.h"
#include "shared_ptr.h"
#include "symbol_table.h"
#include "op_array.h"

#include <boost/random.hpp>

namespace tora {

const int GLOBAL_VAR_ARGV     = 0;
const int GLOBAL_VAR_ENV      = 1;
const int GLOBAL_VAR_LIBPATH  = 2;
const int GLOBAL_VAR_REQUIRED = 3;

class Stack;
class LexicalVarsFrame;
class PackageMap;
class TupleValue;
class Package;
class FunctionFrame;
class CodeValue;

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

class VM {
    ID package_id_;
    SharedPtr<Package> package_; // cached
    bool dump_ops_;
public:
    int sp; // stack pointer
    int pc; // program counter
    SharedPtr<OPArray> ops;
    std::vector<SharedPtr<Value>> *global_vars;
    SharedPtr<SymbolTable> symbol_table;
    SharedPtr<PackageMap> package_map;
    Package* find_package(ID id);
    Package* find_package(const char *name);
    std::vector<SharedPtr<Value>> stack;
    bool exec_trace;

    std::string &package_name() {
        return symbol_table->id2name(package_id_);
    }
    void package_name(const std::string& s) {
        // will be deprecate
        package_id_ = symbol_table->get_id(s);
    }

    bool dump_ops() const { return dump_ops_; }

    const SharedPtr<Package> & package() {
        return package_;
    }

    ID package_id() {
        return package_id_;
    }
    void package_id(ID id);

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

    VM(SharedPtr<OPArray>& ops_, SharedPtr<SymbolTable> &symbol_table_, bool dump_ops);
    ~VM();
    void execute();
    void execute_normal();
    void execute_trace();


    void init_globals(int argc, char**argv);

    void dump_frame();
    void dump_stack();
    SharedPtr<Value> require(Value *v);
    void add_function(ID pkgid, ID id, SharedPtr<Value> code);
    void add_function(ID pkgid, std::string &name, SharedPtr<Value> code) {
        this->add_function(pkgid, this->symbol_table->get_id(name), code);
    }
    void die(const SharedPtr<Value> & exception);
    void die(const char *format, ...);

    void register_standard_methods();

    int get_int_operand() const {
        return ops->at(pc)->operand.int_value;
    }
    int get_int_operand_high() const {
        return (get_int_operand() >> 16) & 0x0000FFFF;
    }
    int get_int_operand_low() const {
        return get_int_operand() & 0x0000ffff;
    }
    double get_double_operand() const {
        return ops->at(pc)->operand.double_value;
    }

    SharedPtr<Value> copy_all_public_symbols(ID srcid, ID dstid);
    SharedPtr<Value> get_self();

    void call_native_func(const CallbackFunction* callback, int argcnt);

    const SharedPtr<Value>& TOP() { return stack.back(); }
    SharedPtr<Value> set_item(const SharedPtr<Value>& container, const SharedPtr<Value>& index, const SharedPtr<Value>& rvalue) const;
    void extract_tuple(const SharedPtr<TupleValue> &t);

    boost::mt19937 *myrand;

    /**
     * Call a function.
     */
    void function_call(int argcnt, const SharedPtr<CodeValue>& code, const SharedPtr<Value> & self);
    /**
     * inject code to VM and run it. And... restore VM state!
     */
    void function_call_ex(int argcnt, const SharedPtr<CodeValue>& code, const SharedPtr<Value> & self);

    /**
     * load dynamic library.
     */
    void load_dynamic_library(const std::string &filename, const std::string &endpoint);

    /**
     * Add new library path.
     */
    void add_library_path(const std::string &dir);

    void call_method(const SharedPtr<Value> &object, const SharedPtr<Value> &function_id);
    void call_method(const SharedPtr<Value> &object, ID klass_id, const SharedPtr<Value> &function_id, std::set<ID> &seen);

#include "vm.ops.inc.h"

private:
    void handle_exception(const SharedPtr<Value> & exception);
    void dump_value(const SharedPtr<Value> & v);
};

};

#endif // VM_H_

