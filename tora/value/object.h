#ifndef TORA_VALUE_OBJECT_T_
#define TORA_VALUE_OBJECT_T_

#include "../value.h"
#include "../package.h"

namespace tora {

class VM;

class ObjectImpl {
    friend class ObjectValue;
protected:
    VM * vm_;
    SharedPtr<Package> package_;
    bool destroyed_;
    SharedPtr<Value> data_;
    ObjectImpl(VM *vm, Package* pkg, const SharedPtr<Value>& d)
        : vm_(vm)
        , package_(pkg)
        , destroyed_(false)
        , data_(d)
        { }
};

class ObjectValue : public Value {
private:
    const ObjectImpl & VAL() const {
        return *object_value_;
    }
    ObjectImpl & VAL() {
        return *object_value_;
    }
public:
    ObjectValue(VM *v, ID pkgid, const SharedPtr<Value>& d);
    ~ObjectValue();
    const SharedPtr<Value> data() const { return VAL().data_; }
    SharedPtr<Value> data() { return VAL().data_; }
    void release();
    void call_destroy();
    VM * vm() const { return VAL().vm_; }

    ID package_id() const { return VAL().package_->id(); }

    void dump(int indent);
    const char *type_str() const;

    SharedPtr<Value> get_item(SharedPtr<Value> index);
    SharedPtr<Value> set_item(SharedPtr<Value>index, SharedPtr<Value>v);

private:
    std::string package_name() const;
};

};

#endif // TORA_VALUE_OBJECT_T_

