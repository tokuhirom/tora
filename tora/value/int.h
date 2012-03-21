#ifndef TORA_VALUE_INT_H_
#define TORA_VALUE_INT_H_

#include "../value.h"

#include <boost/pool/object_pool.hpp>

namespace tora {

class IntValue: public Value {
public:
    IntValue(int i): Value(VALUE_TYPE_INT) {
        this->value_ = i;
    }
    ~IntValue() { }
    void tora__decr__() {
        boost::get<int>(this->value_)--;
    }
    void tora__incr__() {
        boost::get<int>(this->value_)++;
    }
    SharedPtr<IntValue> clone() const {
        return new IntValue(boost::get<int>(this->value_));
    }
    int int_value() const {
        return boost::get<int>(this->value_);
    }
    void int_value(int n) {
        boost::get<int>(this->value_) = n;
    }
public:
	void* operator new(size_t size) { return pool_.malloc(); }
	void operator delete(void* doomed, size_t) { pool_.free((IntValue*)doomed); }
private:
    static boost::object_pool<IntValue> pool_;
};

};

#endif // TORA_VALUE_INT_H_
