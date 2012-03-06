#ifndef TORA_FRAME_H_
#define TORA_FRAME_H_

#include <vector>

#include <boost/pool/object_pool.hpp>

#include "shared_ptr.h"
#include "value.h"

namespace tora {

class OPArray;
class VM;
class CodeValue;

typedef enum {
    FRAME_TYPE_LEXICAL = 1,
    FRAME_TYPE_FUNCTION,
    FRAME_TYPE_TRY,
    FRAME_TYPE_FOREACH,
    FRAME_TYPE_PACKAGE,
    FRAME_TYPE_WHILE,
    FRAME_TYPE_FOR,
} frame_type_t;

// TODO rename LexicalVarsFrame to Frame
class LexicalVarsFrame {
private:
public:
    std::vector<SharedPtr<Value>> vars;
    size_t top;
    frame_type_t type;
    SharedPtr<CodeValue> code;
    LexicalVarsFrame(int vars_cnt, size_t top, frame_type_t type_=FRAME_TYPE_LEXICAL);
    virtual ~LexicalVarsFrame() { }
    void setVar(int id, const SharedPtr<Value>& v) {
        assert(id < this->vars.capacity());
        this->vars[id] = v.get();
    }
    SharedPtr<Value> find(int id) {
        assert(id < this->vars.capacity());
        return this->vars[id];
    }
    const char *type_str() {
        switch (type) {
        case FRAME_TYPE_FUNCTION:
            return "function";
        case FRAME_TYPE_LEXICAL:
            return "lexical";
        case FRAME_TYPE_PACKAGE:
            return "package";
        case FRAME_TYPE_TRY:
            return "try";
        case FRAME_TYPE_FOREACH:
            return "foreach";
        case FRAME_TYPE_WHILE:
            return "while";
        case FRAME_TYPE_FOR:
            return "for";
        }
        abort();
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
    TryFrame(size_t top) : LexicalVarsFrame(0, top, FRAME_TYPE_TRY) { }
};

class FunctionFrame : public LexicalVarsFrame {
public:
    int return_address;
    SharedPtr<OPArray> orig_ops;
    SharedPtr<Value> self;
    int argcnt;
    FunctionFrame(int vars_cnt, size_t top) : LexicalVarsFrame(vars_cnt, top, FRAME_TYPE_FUNCTION) { }
    FunctionFrame(int vars_cnt, size_t top, const SharedPtr<OPArray> & op) : LexicalVarsFrame(vars_cnt, top), orig_ops(op) {
        this->type = FRAME_TYPE_FUNCTION;
    }
    FunctionFrame(int vars_cnt, size_t top, const SharedPtr<Value>& self_) : LexicalVarsFrame(vars_cnt, top), self(self_) { }
    ~FunctionFrame() { }
public:
	void* operator new(size_t size) { return pool_.malloc(); }
	void operator delete(void* doomed, size_t) { pool_.free((FunctionFrame*)doomed); }
private:
    static boost::object_pool<FunctionFrame> pool_;
};

class ForeachFrame : public LexicalVarsFrame {
public:
    SharedPtr<Value> iter;
    ForeachFrame(int vars_cnt, size_t top) : LexicalVarsFrame(vars_cnt, top, FRAME_TYPE_FOREACH) { }
};


class PackageFrame : public LexicalVarsFrame {
    ID orig_package_id_;
    VM * vm_;
public:
    PackageFrame(size_t top, ID pkgid, VM *parent) : LexicalVarsFrame(0, top, FRAME_TYPE_PACKAGE), orig_package_id_(pkgid), vm_(parent) { }
    ~PackageFrame();
};

};

#endif // TORA_FRAME_H_
