#ifndef TORA_OP_ARRAY_H_
#define TORA_OP_ARRAY_H_

#include "op.h"

namespace tora {

class OPArray {
    PRIM_DECL(OPArray)
private:
    std::vector<OP*> ops;
    std::vector<size_t> lineno;
public:
    OPArray() { }
    ~OPArray() {
        auto iter = ops.begin();
        for (; iter!=ops.end(); iter++) {
            delete *iter;
        }
    }
    const OP* at(size_t i) const {
#ifdef NDEBUG
        // operator[] is faster.
        return this->ops[i];
#else
        return this->ops.at(i);
#endif
    }
    const op_type_t op_type_at(size_t i) const {
        return this->ops[i]->op_type;
    }
    size_t size() const {
        return this->ops.size();
    }
    void push_back(OP* o, size_t lineno) {
        o->retain();
        this->ops.push_back(o);
        this->lineno.push_back(lineno);
    }
    void push_back(SharedPtr<ValueOP>& o, size_t lineno) {
        o->retain();
        this->ops.push_back(&(*(o)));
        this->lineno.push_back(lineno);
    }
    void push_back(const SharedPtr<OP>& o, size_t lineno) {
        o->retain();
        this->ops.push_back(&(*(o)));
        this->lineno.push_back(lineno);
    }

    size_t get_lineno(size_t pc) {
        return lineno[pc];
    }
};

};

#endif // TORA_OP_ARRAY_H_
