#include <stdlib.h>

#include "../vm.h"
#include "../object.h"
#include "../package.h"
#include "../value/array.h"
#include "../value/hash.h"

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
    SharedPtr<ArrayValue> av = new ArrayValue();
    for (auto iter=self->upcast<HashValue>()->begin(); iter!=self->upcast<HashValue>()->end(); ++iter) {
        av->push_back(new StrValue(iter->first));
    }
    return av;
}

/**
 * $hash.values() : Array[Any]
 *
 * Return values from $hash.
 */
static SharedPtr<Value> Hash_values(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_HASH);
    SharedPtr<ArrayValue> av = new ArrayValue();
    for (auto iter=self->upcast<HashValue>()->begin(); iter!=self->upcast<HashValue>()->end(); ++iter) {
        av->push_back(iter->second);
    }
    return av;
}

/**
 * $hash.delete(String $key) : Undef
 *
 * Delete $key from $hash.
 */
static SharedPtr<Value> Hash_delete(VM * vm, Value* self, Value *key) {
    assert(self->value_type == VALUE_TYPE_HASH);
    self->upcast<HashValue>()->delete_key(key->to_s());
    return UndefValue::instance();
}

/**
 * $hash.exists(String $key) : Boolean
 *
 * Check hash key existence.
 */
static SharedPtr<Value> Hash_exists(VM * vm, Value* self, Value *key) {
    assert(self->value_type == VALUE_TYPE_HASH);
    bool b = self->upcast<HashValue>()->has_key(key->to_s());
    return new BoolValue(b);
}

void tora::Init_Hash(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("Hash");
    pkg->add_method("keys",   new CallbackFunction(Hash_keys));
    pkg->add_method("values", new CallbackFunction(Hash_values));
    pkg->add_method("delete", new CallbackFunction(Hash_delete));
    pkg->add_method("exists", new CallbackFunction(Hash_exists));
}

