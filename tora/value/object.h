#ifndef TORA_VALUE_OBJECT_T_
#define TORA_VALUE_OBJECT_T_

#include "../value.h"

namespace tora {

  class Value;
  class SharedValue;
  class VM;
  class MortalValue;

  class MortalObjectValue : public MortalValue {
    static Value* new_value(VM*vm, Value* klass, Value* data);
  public:
    MortalObjectValue(VM* vm, Value* klass, Value* data)
      : MortalValue(new_value(vm, klass, data)) { }
  };

  SharedValue object_get_item(Value* self, Value* index);
  SharedValue object_set_item(Value* self, Value* index, Value* v);
  bool object_isa(Value* self, ID target_id);
  void object_free(Value* self);
  void object_finalize(Value* self);
  Value* object_data(Value* self);
  Value* object_class(const Value* self);
  std::string object_type_str(const Value* self);
  void object_dump(Value* self, int indent);

};

#endif  // TORA_VALUE_OBJECT_T_
