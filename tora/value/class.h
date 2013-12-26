#ifndef TORA_VALUE_CLASS_H_
#define TORA_VALUE_CLASS_H_

#include "../value.h"
#include "../callback.h"

namespace tora {

  class VM;

  class MortalClassValue : public MortalValue {
    static Value* new_value(VM* vm, ID name_id);
  public:
    MortalClassValue(VM* vm, ID name_id)
      : MortalValue(new_value(vm, name_id)) {
    }
  };

  void class_free(Value* self);
  ID class_name_id(Value* self);
  std::string class_name(const Value* self);
  void class_add_constant(Value* self, ID name_id, int val);
  void class_add_method(Value* self, Value* code);
  void class_add_method(Value* self, ID name_id, const CallbackFunction* cb);
  bool class_has_method(Value* self, ID target_id);
  SharedValue class_get_method_list(Value* self);
  bool class_isa(Value* self, ID target_id);
  Value* class_get_method(Value *self, ID id);
  SharedValue class_superclass(Value* self);
  void class_superclass(Value* v, Value* super);

}

#endif  // TORA_VALUE_CLASS_H_
