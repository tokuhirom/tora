#ifndef TORA_VALUE_OBJECT_T_
#define TORA_VALUE_OBJECT_T_

#include "../value.h"

namespace tora {

class VM;

class ObjectValue : public Value {
private:
    const ObjectImpl & VAL() const {
        return *object_value_;
    }
    ObjectImpl & VAL() {
        return *object_value_;
    }
public:
    ObjectValue(VM *v, ID p, const SharedPtr<Value>& d) : Value(VALUE_TYPE_OBJECT) {
        object_value_ = new ObjectImpl(v, p, d);
    }
    ~ObjectValue();
    const SharedPtr<Value> data() const { return VAL().data_; }
    void release();
    void call_destroy();

    ID package_id() const { return VAL().package_id_; }

    void dump(int indent);
    const char *type_str() const;

    SharedPtr<Value> get_item(SharedPtr<Value> index);
    SharedPtr<Value> set_item(SharedPtr<Value>index, SharedPtr<Value>v);
};

};

#endif // TORA_VALUE_OBJECT_T_

