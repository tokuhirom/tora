#ifndef TORA_VALUE_INT_H_
#define TORA_VALUE_INT_H_

#include "../value.h"

#include <boost/pool/object_pool.hpp>

namespace tora {

class IntValue: public Value {
public:
    IntValue(int i): Value(VALUE_TYPE_INT) {
        this->int_value_ = i;
    }
    ~IntValue() { }
    void tora__decr__() {
        this->int_value_--;
    }
    void tora__incr__() {
        this->int_value_++;
    }
    SharedPtr<IntValue> clone() const {
        return new IntValue(this->int_value_);
    }
    int int_value() const {
        return this->int_value_;
    }
    void int_value(int n) {
        this->int_value_ = n;
    }
public:
	void* operator new(size_t size) { return pool_.malloc(); }
	void operator delete(void* doomed, size_t) { pool_.free((IntValue*)doomed); }
private:
    static boost::object_pool<IntValue> pool_;
};

};

#endif // TORA_VALUE_INT_H_
