#ifndef TORA_VALUE_OBJECT_T_
#define TORA_VALUE_OBJECT_T_

#include "../value.h"

namespace tora {

class VM;

class ObjectValue : public Value {
    VM * vm_;
    ID package_id_;
    bool destroyed;
    SharedPtr<Value> data_;
public:
    ObjectValue(VM *v, ID p, const SharedPtr<Value>& d) : Value(VALUE_TYPE_OBJECT), vm_(v), package_id_(p), destroyed(false), data_(d) {
    }
    ~ObjectValue();
    const SharedPtr<Value> data() { return data_; }
    void release();
    void call_destroy();

    ID package_id() { return package_id_; }

    void dump(int indent);
    const char *type_str() const;

    SharedPtr<Value> get_item(SharedPtr<Value> index);
    SharedPtr<Value> set_item(SharedPtr<Value>index, SharedPtr<Value>v);
};

};

#endif // TORA_VALUE_OBJECT_T_

