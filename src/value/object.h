#ifndef TORA_VALUE_OBJECT_T_
#define TORA_VALUE_OBJECT_T_

#include "../value.h"

namespace tora {

class VM;

class ObjectValue : public Value {
    std::map<ID, SharedPtr<Value>> data;
    ID package_id_;
    VM * vm_;
public:
    ObjectValue(ID p, VM *v) : Value(VALUE_TYPE_OBJECT), package_id_(p), vm_(v) {
    }
    ~ObjectValue();

    ID package_id() { return package_id_; }

    void set_value(ID id, const SharedPtr<Value> & v) { data[id] = v; }
    SharedPtr<Value> get_value(ID id) { return data[id]; }

    void dump(int indent) {
        print_indent(indent);
        printf("[dump] Object: %d\n", package_id_);
    }
    const char *type_str() { return "object"; }

    SharedPtr<Value> get_item(SharedPtr<Value> index);
    Value* set_item(SharedPtr<Value>index, SharedPtr<Value>v);
};

};

#endif // TORA_VALUE_OBJECT_T_

