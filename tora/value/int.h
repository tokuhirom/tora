#ifndef TORA_VALUE_INT_H_
#define TORA_VALUE_INT_H_

#include "../value.h"

#ifdef ENABLE_OBJECT_POOL
#include <boost/pool/object_pool.hpp>
#endif

namespace tora {

class IntValue: public Value {
private:
    const int &VAL() const {
        return this->int_value_;
    }
    int &VAL() {
        return this->int_value_;
    }
public:
    IntValue(int i): Value(VALUE_TYPE_INT) {
        this->int_value_ = i;
    }
    ~IntValue() { }
    // remove?
    void tora__decr__() {
        VAL()--;
    }
    SharedPtr<IntValue> clone() const {
        return new IntValue(VAL());
    }
    // remove?
    void tora__incr__() {
        VAL()++;
    }
    int int_value() const {
        return VAL();
    }
    void int_value(int n) {
        VAL() = n;
    }
#ifdef ENABLE_OBJECT_POOL
public:
	void* operator new(size_t size) { return pool_.malloc(); }
	void operator delete(void* doomed, size_t) { pool_.free((IntValue*)doomed); }
private:
    static boost::object_pool<IntValue> pool_;
#endif
};

};

#endif // TORA_VALUE_INT_H_
