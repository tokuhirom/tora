#ifndef TORA_FRAME_H_
#define TORA_FRAME_H_

#include <vector>

#include <boost/pool/object_pool.hpp>
#include <boost/shared_ptr.hpp>

#include "shared_ptr.h"
#include "inspector.h"
#include "value.h"
#include "value/code.h"
#include "value/array.h"
#include "pad_list.h"

namespace tora {

class OPArray;
class VM;
class CodeValue;
class Package;
class ArrayValue;
class PadList;

typedef enum {
    FRAME_TYPE_LEXICAL = 1,
    FRAME_TYPE_FUNCTION,
    FRAME_TYPE_TRY,
    FRAME_TYPE_FOREACH,
    FRAME_TYPE_PACKAGE,
    FRAME_TYPE_WHILE,
    FRAME_TYPE_FOR,
} frame_type_t;

struct DynamicScopeData {
    PRIM_DECL(DynamicScopeData);
    Package * package_;
    ID moniker_id_;
    SharedPtr<Value> value_;
public:
    DynamicScopeData(Package * pkg, ID m, const SharedPtr<Value> & val) :refcnt(0), package_(pkg), moniker_id_(m), value_(val) {
    }
    ID moniker_id() const { return moniker_id_; }
    Package * package() const { return package_; }
    SharedPtr<Value> value() const { return value_; }
};


// TODO rename LexicalVarsFrame to Frame
class LexicalVarsFrame {
    PRIM_DECL(LexicalVarsFrame);
protected:
    VM * vm_;
    std::vector<SharedPtr<Value>> vars;
public:
    size_t top;
    frame_type_t type;
    SharedPtr<CodeValue> code;
    std::vector<SharedPtr<DynamicScopeData>> dynamic_scope_vars;
    SharedPtr<PadList> pad_list;

    LexicalVarsFrame(VM *vm, int vars_cnt, size_t top, frame_type_t type_=FRAME_TYPE_LEXICAL);
    // this 'virtual' is required for memory pool.
    virtual ~LexicalVarsFrame();
    void set_variable_dynamic(int level, int id, const SharedPtr<Value>& v);
    void set_variable(int id, const SharedPtr<Value>& v);
    SharedPtr<Value> get_variable(int id) const {
        assert(id < this->vars.capacity());
        return this->pad_list->get(id);
    }
    void push_dynamic_scope_var(Package* pkgid, ID monikerid, const SharedPtr<Value> &target);
    const char *type_str() const;
    SharedPtr<Value> get_variable_dynamic(int level, int no) const;

    template<class Y>
    Y* upcast() {
        return static_cast<Y*>(this);
    }

};

class TryFrame : public LexicalVarsFrame {
public:
    int return_address;
    // try frame does not have variables.
    TryFrame(VM *vm, size_t top) : LexicalVarsFrame(vm, 0, top, FRAME_TYPE_TRY) { }
};

class FunctionFrame : public LexicalVarsFrame {
public:
    int return_address;
    SharedPtr<OPArray> orig_ops;
    SharedPtr<Value> self;
    int argcnt;
    FunctionFrame(VM *vm, int vars_cnt, size_t top) : LexicalVarsFrame(vm, vars_cnt, top, FRAME_TYPE_FUNCTION) { }
    FunctionFrame(VM *vm, int vars_cnt, size_t top, const SharedPtr<OPArray> & op) : LexicalVarsFrame(vm, vars_cnt, top), orig_ops(op) {
        this->type = FRAME_TYPE_FUNCTION;
    }
    FunctionFrame(VM *vm, int vars_cnt, size_t top, const SharedPtr<Value>& self_) : LexicalVarsFrame(vm, vars_cnt, top), self(self_) { }
public:
	void* operator new(size_t size) { return pool_.malloc(); }
	void operator delete(void* doomed, size_t) { pool_.free((FunctionFrame*)doomed); }
private:
    static boost::object_pool<FunctionFrame> pool_;
};

class ForeachFrame : public LexicalVarsFrame {
public:
    SharedPtr<Value> iter;
    SharedPtr<Value> current_value;
    ForeachFrame(VM *vm, int vars_cnt, size_t top) : LexicalVarsFrame(vm, vars_cnt, top, FRAME_TYPE_FOREACH) { }
};


class PackageFrame : public LexicalVarsFrame {
    friend class LexicalVarsFrame;
protected:
    ID orig_package_id_;
public:
    PackageFrame(VM *vm, size_t top, ID pkgid) : LexicalVarsFrame(vm, 0, top, FRAME_TYPE_PACKAGE), orig_package_id_(pkgid) { }
};

};

#endif // TORA_FRAME_H_
