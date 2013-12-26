#ifndef TORA_VALUE_OBJECT_T_
#define TORA_VALUE_OBJECT_T_

#include "../value.h"

namespace tora {

class VM;

class ObjectImpl {
  friend class ObjectValue;

 protected:
  VM* vm_;
  SharedPtr<Value> klass_;
  bool destroyed_;
  SharedPtr<Value> data_;
  ObjectImpl(VM* vm, const SharedPtr<Value>& klass,
             const SharedPtr<Value>& d)
      : vm_(vm), klass_(klass), destroyed_(false), data_(d) {}
};

class ObjectValue : public Value {
 private:
  const ObjectImpl& VAL() const { return *object_value_; }
  ObjectImpl& VAL() { return *object_value_; }

 public:
  ObjectValue(VM* v, const SharedPtr<Value>& klass,
              const SharedPtr<Value>& data);
  ObjectValue(VM* v, ID klass_id, const SharedPtr<Value>& data);
  ~ObjectValue();
  const SharedPtr<Value> data() const { return VAL().data_; }
  SharedPtr<Value> data() { return VAL().data_; }
  void release();
  void call_destroy();
  VM* vm() const { return VAL().vm_; }

  SharedPtr<Value> class_value() const { return VAL().klass_; }

  void dump(int indent);
  const char* type_str() const;

  SharedPtr<Value> get_item(SharedPtr<Value> index);
  SharedPtr<Value> set_item(SharedPtr<Value> index, SharedPtr<Value> v);
  bool isa(ID target_id) const;
};
};

#endif  // TORA_VALUE_OBJECT_T_
