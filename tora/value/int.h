#ifndef TORA_VALUE_INT_H_
#define TORA_VALUE_INT_H_

#include "../value.h"

#include <boost/pool/object_pool.hpp>

namespace tora {

class IntValue: public Value {
public:
    int  int_value;
    IntValue(int i): Value(VALUE_TYPE_INT) {
        this->int_value = i;
    }
    ~IntValue() { }
    void tora__decr__() {
        this->int_value--;
    }
    void tora__incr__() {
        this->int_value++;
    }
    SharedPtr<IntValue> clone() {
        return new IntValue(this->int_value);
    }
public:
	void* operator new(size_t size) { return pool_.malloc(); }
	void operator delete(void* doomed, size_t) { pool_.free((IntValue*)doomed); }
private:
    static boost::object_pool<IntValue> pool_;
};

};

#endif // TORA_VALUE_INT_H_
