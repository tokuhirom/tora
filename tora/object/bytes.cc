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

/**
 * $bytes.substr(Int $start)              : Str
 * $bytes.substr(Int $start, Int $length) : Str
 *
 * It returns substring from $bytes.
 *
 * Example:
 *   b"foobar".substr(3) # => "bar"
 *   b"foobar".substr(3,2) # => "ba"
 *
 * Get a substring from bytes.
 */
static SharedPtr<Value> Bytes_substr(VM *vm, const std::vector<SharedPtr<Value>>& args) {
    assert(args[0]->value_type == VALUE_TYPE_BYTES);
    const std::string & str = args[0]->upcast<BytesValue>()->str_value();
    if (args.size()-1 == 1) { // "foobar".substr(3)
        return new BytesValue(str.substr(args[1]->to_int()));
    } else if (args.size()-1 == 2) { // "foobar".substr(3,2)
        return new BytesValue(str.substr(args[1]->to_int(), args[2]->to_int()));
    } else {
        throw new ArgumentExceptionValue("Bytes#substr requires 1 or 2 arguments");
    }
}

void tora::Init_Bytes(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("Bytes");
    pkg->add_method(vm->symbol_table->get_id("length"), new CallbackFunction(bytes_length));
    pkg->add_method("substr", new CallbackFunction(Bytes_substr));
}

