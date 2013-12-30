#include "class.h"
#include "code.h"
#include "../vm.h"
#include <utility>

using namespace tora;

struct ClassImpl {
public:
  VM* vm;
  ID name_id;
  SharedValue superclass;
  std::map<ID, SharedValue> methods;
};

Value* MortalClassValue::new_value(VM* vm, ID name_id)
{
  ClassImpl* impl = new ClassImpl();
  impl->name_id = name_id;
  impl->vm      = vm;

  Value* v = new Value(VALUE_TYPE_CLASS);
  v->ptr_value_ = impl;
  return v;
}

static ClassImpl* klass(const Value* v) {
  assert(type(v) == VALUE_TYPE_CLASS);
  return static_cast<ClassImpl*>(v->ptr_value_);
}

// -- impl ---

void tora::class_free(Value* self)
{
  delete klass(self);
}

ID tora::class_name_id(Value* self)
{
  return klass(self)->name_id;
}

std::string tora::class_name(const Value* self)
{
  return klass(self)->vm->symbol_table->id2name(klass(self)->name_id);
}

void tora::class_add_constant(Value* self, ID name_id, int val)
{
  tora::class_add_method(self, name_id, new CallbackFunction(val));
}

void tora::class_add_method(Value* self, Value* code)
{
  klass(self)->methods.insert(
    std::pair<ID, SharedValue>(
      code_func_name_id(code),
      code
    )
  );
}

void tora::class_add_method(Value* self, ID name_id, const CallbackFunction* cb)
{
  klass(self)->methods.insert(
    std::pair<ID, SharedValue>(
      name_id,
      new CodeValue(klass(self)->name_id, name_id, cb)
    )
  );
}

bool tora::class_has_method(Value* self, ID target_id) {
  return klass(self)->methods.find(target_id) != klass(self)->methods.end();
}

SharedValue tora::class_get_method_list(Value* self)
{
  MortalArrayValue av;
  for (auto iter: klass(self)->methods) {
    MortalStrValue s(klass(self)->vm->symbol_table->id2name(iter.first));
    array_push_back(av.get(), s.get());
  }
  return av.get();
}

bool tora::class_isa(Value* self, ID target_id) {
  Value *k = self;
  while (k) {
    if (klass(k)->name_id == target_id) {
      return true;
    }
    k = klass(k)->superclass.get();
  }
  return false;
}

/**
 * Return borrowed value.
 */
Value* tora::class_get_method(Value *self, ID id)
{
  auto iter = klass(self)->methods.find(id);
  if (iter == klass(self)->methods.end()) {
    return NULL;
  } else {
    return iter->second.get();
  }
}

SharedValue tora::class_superclass(Value* self)
{
  return klass(self)->superclass.get();
}

void tora::class_superclass(Value *self, Value* super)
{
  klass(self)->superclass.reset(super);
}

