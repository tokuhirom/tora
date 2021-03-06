#ifndef TORA_VM_H_
#define TORA_VM_H_

#include <vector>
#include <map>
#include <set>
#include <stdarg.h>
#include <stdio.h>

#if (defined(__MINGW32__) || defined(__MINGW64__)) && (__GNUC__ == 4)
// workaround a mingw bug, http://sourceforge.net/tracker/index.php?func=detail&aid=2373234&group_id=2435&atid=102435
int swprintf (wchar_t *, size_t, const wchar_t *, ...);
#endif

#include "tora.h"
#include "op.h"
#include "value.h"
#include "shared_ptr.h"
#include "symbol_table.h"
#include "op_array.h"
#include "callback.h"

namespace tora {

const int GLOBAL_VAR_ARGV     = 0;
const int GLOBAL_VAR_ENV      = 1;
const int GLOBAL_VAR_LIBPATH  = 2;
const int GLOBAL_VAR_REQUIRED = 3;

class Stack;
class LexicalVarsFrame;
class TupleValue;
class FunctionFrame;
class CodeValue;
class ClassValue;

class VM;

class VM {
    friend class LexicalVarsFrame;

    SharedPtr<ClassValue> klass_;
    bool dump_ops_;
    typedef std::map<ID, SharedPtr<Value>> file_scope_body_t;
    std::shared_ptr<file_scope_body_t> file_scope_;
    std::map<ID, std::shared_ptr<std::map<ID, SharedPtr<Value>>>> file_scope_map_;
    std::map<ID, SharedPtr<ClassValue>> builtin_classes_;
    std::map<ID, SharedPtr<CodeValue>> builtin_functions_;
public:
    int sp; // stack pointer
    int pc; // program counter
    SharedPtr<OPArray> ops;
    std::vector<SharedPtr<Value>> *global_vars;
    SharedPtr<SymbolTable> symbol_table;
    std::vector<SharedPtr<Value>> stack;
    bool exec_trace;

    bool dump_ops() const { return dump_ops_; }

    const SharedPtr<ClassValue>& get_builtin_class(ID name_id) const;

    SharedPtr<ClassValue> get_class(ID name_id) const;

    std::string id2name(ID id) const;

    const SharedPtr<ClassValue>& klass() {
        return klass_;
    }
    void klass(const SharedPtr<Value>& k);

    void add_builtin_function(const char *name, CallbackFunction* func);

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


    void init_globals(const std::vector<std::string> & args);

    void dump_frame();
    void dump_stack();
    void use(Value *v, bool);
    void require_package(const std::string &);
    void add_function(const SharedPtr<CodeValue>& klass);
    void add_function(ID id, const CallbackFunction *cb);
    void add_function(const char *name, const CallbackFunction *cb);
    void add_constant(const char *name, int n);
    void add_class(const SharedPtr<ClassValue>& code);
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

    void add_builtin_class(const SharedPtr<ClassValue>& klass);

    SharedPtr<Value> get_self();

    const SharedPtr<Value>& TOP() { return stack.back(); }
    SharedPtr<Value> set_item(const SharedPtr<Value>& container, const SharedPtr<Value>& index, const SharedPtr<Value>& rvalue) const;
    void extract_tuple(const SharedPtr<TupleValue> &t);

    std::mt19937 *myrand;

    /**
     * Call a function.
     */
    void function_call(int argcnt, const SharedPtr<CodeValue>& code, const SharedPtr<Value> & self);
    /**
     * inject code to VM and run it. And... restore VM state!
     *
     * You must take a return value from stack top.
     */
    void function_call_ex(int argcnt, const SharedPtr<CodeValue>& code, const SharedPtr<Value> & self);

    /**
     * eval the source code from is.
     */
    SharedPtr<Value> eval(std::istream* is, const std::string & fname);

    /**
     * load dynamic library.
     */
    void load_dynamic_library(const std::string &filename, const std::string &endpoint);

    /**
     * Add new library path.
     */
    void add_library_path(const std::string &dir);

    void call_method(const SharedPtr<Value> &object, const SharedPtr<Value> &function_id);
    void call_method(const SharedPtr<Value> &object, const SharedPtr<ClassValue>& klass_id, ID function_id, std::set<ID> &seen);

    void dump_value(const SharedPtr<Value> & v);
    void dump_pad();

    ID get_id(const std::string &name) const;

#include "vm.ops.inc.h"

private:
    void handle_exception(const SharedPtr<Value> & exception);
    void call_native_func(const CallbackFunction* callback, int argcnt);
    void copy_all_public_symbols(ID srcid);
};

};

#endif // TORA_VM_H_

