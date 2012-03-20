#include <stdlib.h>

#include "../vm.h"
#include "../object.h"
#include "../package.h"
#include "../value/regexp.h"

using namespace tora;

/**
 * $re.flags() : Int
 *
 * Return the flags.
 */
static SharedPtr<Value> Regexp_flags(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_REGEXP);
    return new IntValue(self->upcast<AbstractRegexpValue>()->flags());
}

/**
 * Regexp.MULTILINE : Int
 *
 * multiline flag.
 */
static SharedPtr<Value> Regexp_MULTILINE(VM * vm, Value* self) {
    return new IntValue(REGEXP_MULTILINE);
}

/**
 * Regexp.IGNORECASE : Int
 *
 * case insensitive flag.
 */
static SharedPtr<Value> Regexp_IGNORECASE(VM * vm, Value* self) {
    return new IntValue(REGEXP_IGNORECASE);
}

/**
 * Regexp.EXPANDED : Int
 *
 * //x flag.
 */
static SharedPtr<Value> Regexp_EXPANDED(VM * vm, Value* self) {
    return new IntValue(REGEXP_EXPANDED);
}

void tora::Init_Regexp(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("Regexp");
    pkg->add_method(vm->symbol_table->get_id("flags"), new CallbackFunction(Regexp_flags));
    pkg->add_method(vm->symbol_table->get_id("MULTILINE"), new CallbackFunction(Regexp_MULTILINE));
    pkg->add_method(vm->symbol_table->get_id("IGNORECASE"), new CallbackFunction(Regexp_IGNORECASE));
    pkg->add_method(vm->symbol_table->get_id("EXPANDED"), new CallbackFunction(Regexp_EXPANDED));
}

