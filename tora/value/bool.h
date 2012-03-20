#ifndef TORA_VALUE_BOOL_H_
#define TORA_VALUE_BOOL_H_

#include "../value.h"

#include <boost/pool/object_pool.hpp>

namespace tora {

class BoolValue: public Value {
public:
    BoolValue(bool b): Value(VALUE_TYPE_BOOL) {
        this->bool_value = b;
    }
    bool bool_value;
    static BoolValue* true_instance() {
        return new BoolValue(true);
    }
    static BoolValue* false_instance() {
        return new BoolValue(false);
    }
    static BoolValue* instance(bool b) {
        return b ? BoolValue::true_instance() : BoolValue::false_instance();
    }
public:
	void* operator new(size_t size) { return pool_.malloc(); }
	void operator delete(void* doomed, size_t) { pool_.free((BoolValue*)doomed); }
private:
    static boost::object_pool<BoolValue> pool_;
};

};

#endif // TORA_VALUE_BOOL_H_
