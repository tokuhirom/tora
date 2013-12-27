#include <stdlib.h>

#include "../vm.h"
#include "../object.h"
#include "../symbols.gen.h"
#include "../value/regexp.h"
#include "../value/class.h"
#include "../value/int.h"
#include "../class_builder.h"

using namespace tora;

/**
 * class Regexp
 *
 * Regular expression object.
 */

/**
 * $re.flags() : Int
 *
 * Return the flags.
 */
static SharedPtr<Value> Regexp_flags(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_REGEXP);
  MortalIntValue miv(self->upcast<AbstractRegexpValue>()->flags()); return miv.get();
}

/**
 * Regexp.quotemeta(Str $val) : Str
 *
 * quote meta character in $val and return escaped string.
 */
static SharedPtr<Value> Regexp_quotemeta(VM* vm, Value* self, Value* val) {
  return new_str_value(RE2RegexpValue::quotemeta(val->to_s()));
}

/**
 * Regexp.MULTILINE : Int
 *
 * multiline flag.
 */
static SharedPtr<Value> Regexp_MULTILINE(VM* vm, Value* self) {
  MortalIntValue miv(REGEXP_MULTILINE); return miv.get();
}

/**
 * Regexp.IGNORECASE : Int
 *
 * case insensitive flag.
 */
static SharedPtr<Value> Regexp_IGNORECASE(VM* vm, Value* self) {
  MortalIntValue miv(REGEXP_IGNORECASE); return miv.get();
}

/**
 * Regexp.EXPANDED : Int
 *
 * //x flag.
 */
static SharedPtr<Value> Regexp_EXPANDED(VM* vm, Value* self) {
  MortalIntValue miv(REGEXP_EXPANDED); return miv.get();
}

void tora::Init_Regexp(VM* vm) {
  ClassBuilder builder(vm, SYMBOL_REGEXP_CLASS);
  builder.add_method("flags", new CallbackFunction(Regexp_flags));
  builder.add_method("quotemeta", new CallbackFunction(Regexp_quotemeta));

  builder.add_method("MULTILINE", new CallbackFunction(Regexp_MULTILINE));
  builder.add_method("IGNORECASE", new CallbackFunction(Regexp_IGNORECASE));
  builder.add_method("EXPANDED", new CallbackFunction(Regexp_EXPANDED));
  vm->add_builtin_class(builder.value());
}
