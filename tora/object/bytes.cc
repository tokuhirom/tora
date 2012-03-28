#include <stdlib.h>

#include "../vm.h"
#include "../object.h"
#include "../package.h"
#include "../value/bytes.h"

using namespace tora;

/**
 * class Bytes
 *
 * Bytes class is character set independent byte string class.
 */

/**
 * $bytes.length() : Int
 *
 * Return the caller code object.
 */
static SharedPtr<Value> bytes_length(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_BYTES);
    return new IntValue(self->upcast<BytesValue>()->length());
}

void tora::Init_Bytes(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("Bytes");
    pkg->add_method(vm->symbol_table->get_id("length"), new CallbackFunction(bytes_length));
}

