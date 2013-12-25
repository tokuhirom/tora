#include <stdlib.h>

#include "../vm.h"
#include "../object.h"
#include "../symbols.gen.h"
#include "../value/array.h"
#include "../value/hash.h"
#include "../value/class.h"

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
static SharedPtr<Value> Hash_keys(VM * vm, Value* self) {
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
static SharedPtr<Value> Hash_values(VM * vm, Value* self) {
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
static SharedPtr<Value> Hash_delete(VM * vm, Value* self, Value *key) {
    assert(self->value_type == VALUE_TYPE_HASH);
    hash_delete_key(self, key->to_s());
    return new_undef_value();
}

/**
 * $hash.exists(String $key) : Boolean
 *
 * Check hash key existence.
 */
static SharedPtr<Value> Hash_exists(VM * vm, Value* self, Value *key) {
    assert(self->value_type == VALUE_TYPE_HASH);
    bool b = hash_has_key(self, key->to_s());
    return vm->to_bool(b);
}

void tora::Init_Hash(VM *vm) {
    SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_HASH_CLASS);
    klass->add_method("keys",   new CallbackFunction(Hash_keys));
    klass->add_method("values", new CallbackFunction(Hash_values));
    klass->add_method("delete", new CallbackFunction(Hash_delete));
    klass->add_method("exists", new CallbackFunction(Hash_exists));
    vm->add_builtin_class(klass);
}

