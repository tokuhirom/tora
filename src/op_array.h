#ifndef TORA_OP_ARRAY_H_
#define TORA_OP_ARRAY_H_

#include "op.h"

namespace tora {

class OPArray : public Prim {
private:
    std::vector<OP*> *ops;
public:
    OPArray() {
        ops = new std::vector<OP*>();
    }
    ~OPArray() {
        auto iter = ops->begin();
        for (; iter!=ops->end(); iter++) {
            delete *iter;
        }
        delete ops;
    }
    OP* at(size_t i) {
        return this->ops->at(i);
    }
    size_t size() {
        return this->ops->size();
    }
    void push_back(OP* o) {
        o->retain();
        this->ops->push_back(o);
    }
    void push_back(SharedPtr<ValueOP>& o) {
        o->retain();
        this->ops->push_back(&(*(o)));
    }
    void push_back(SharedPtr<OP>& o) {
        o->retain();
        this->ops->push_back(&(*(o)));
    }
};

};

#endif // TORA_OP_ARRAY_H_
