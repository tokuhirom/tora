#ifndef TORA_OP_ARRAY_H_
#define TORA_OP_ARRAY_H_

#include "op.h"

namespace tora {

class OPArray : public Prim {
private:
    std::vector<OP*> ops;
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
    size_t size() const {
        return this->ops.size();
    }
    void push_back(OP* o) {
        o->retain();
        this->ops.push_back(o);
    }
    void push_back(SharedPtr<ValueOP>& o) {
        o->retain();
        this->ops.push_back(&(*(o)));
    }
    void push_back(SharedPtr<OP>& o) {
        o->retain();
        this->ops.push_back(&(*(o)));
    }
};

};

#endif // TORA_OP_ARRAY_H_
