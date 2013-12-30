#include <stdlib.h>

#include "../vm.h"
#include "../object.h"
#include "../symbols.gen.h"
#include "../value/array.h"
#include "../value/hash.h"
#include "../value/class.h"
#include "../class_builder.h"

using namespace tora;

/**
 * class Hash
 *
 * Hash is a associate array class.
 */

/**
 * $hash.keys() : Array[Str]
 *
 * Return keys from $hash.
 */
static SharedPtr<Value> Hash_keys(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_HASH);
  MortalArrayValue av;
  MortalHashIteratorValue iter(self);
  while (!hash_iter_finished(iter.get())) {
    MortalStrValue s(hash_iter_getkey(iter.get()));
    array_push_back(av.get(), s.get());
    hash_iter_increment(iter.get());
  }
  return av.get();
}

/**
 * $hash.values() : Array[Any]
 *
 * Return values from $hash.
 */
static SharedPtr<Value> Hash_values(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_HASH);
  MortalArrayValue av;
  MortalHashIteratorValue iter(self);
  while (!hash_iter_finished(iter.get())) {
    array_push_back(av.get(), hash_iter_getval(iter.get()));
    hash_iter_increment(iter.get());
  }
  return av.get();
}

/**
 * $hash.delete(String $key) : Undef
 *
 * Delete $key from $hash.
 */
static SharedPtr<Value> Hash_delete(VM* vm, Value* self, Value* key) {
  assert(self->value_type == VALUE_TYPE_HASH);
  hash_delete_key(self, key->to_s());
  return new_undef_value();
}

/**
 * $hash.exists(String $key) : Boolean
 *
 * Check hash key existence.
 */
static SharedPtr<Value> Hash_exists(VM* vm, Value* self, Value* key) {
  assert(self->value_type == VALUE_TYPE_HASH);
  bool b = hash_has_key(self, key->to_s());
  return vm->to_bool(b);
}

void tora::Init_Hash(VM* vm) {
  ClassBuilder builder(vm, SYMBOL_HASH_CLASS);
  builder.add_method("keys", std::make_shared<CallbackFunction>(Hash_keys));
  builder.add_method("values", std::make_shared<CallbackFunction>(Hash_values));
  builder.add_method("delete", std::make_shared<CallbackFunction>(Hash_delete));
  builder.add_method("exists", std::make_shared<CallbackFunction>(Hash_exists));
  vm->add_builtin_class(builder.value());
}
