#ifndef TORA_VALUE_OBJECT_T_
#define TORA_VALUE_OBJECT_T_

#include "../value.h"

namespace tora {

class ObjectValue : public Value {
    std::map<ID, SharedPtr<Value>> data;
    ID package_id_;
public:
    ObjectValue(ID p) : Value(VALUE_TYPE_OBJECT), package_id_(p) {
    }
    ~ObjectValue() { }

    ID package_id() { return package_id_; }

    void dump(int indent) {
        print_indent(indent);
        printf("[dump] Object: %d\n", package_id_);
    }
    const char *type_str() { return "object"; }
};

};

#endif // TORA_VALUE_OBJECT_T_

