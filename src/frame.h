#ifndef TORA_FRAME_H_
#define TORA_FRAME_H_

namespace tora {

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
    void setVar(int id, const SharedPtr<Value>& v) {
        assert(id < this->vars->capacity());
        (*this->vars)[id] = v.get();
    }
    SharedPtr<Value> find(int id) {
        assert(id < this->vars->capacity());
        return (*this->vars)[id];
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
    void dump(int i) {
        printf("type: %s [%d]\n", this->type_str(), i);
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
    ~FunctionFrame() { }
};

class ForeachFrame : public LexicalVarsFrame {
public:
    SharedPtr<Value> iter;
    ForeachFrame(int vars_cnt, SharedPtr<LexicalVarsFrame> up_) : LexicalVarsFrame(vars_cnt, up_) {
        this->type = FRAME_TYPE_FOREACH;
    }
};


class PackageFrame : public LexicalVarsFrame {
    ID orig_package_id_;
    VM * vm_;
public:
    PackageFrame(ID pkgid, VM *parent, SharedPtr<LexicalVarsFrame> up_) : LexicalVarsFrame(0, up_) {
        this->type = FRAME_TYPE_PACKAGE;
        orig_package_id_ = pkgid;
        vm_ = parent;
    }
    ~PackageFrame() {
        // printf("LEAVE PACKAGE FROM %s. back to %s\n", vm_->package.c_str(), orig_package.c_str());
        vm_->package_id(orig_package_id_);
    }
};

};

#endif // TORA_FRAME_H_
