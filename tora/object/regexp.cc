#include <stdlib.h>

#include "../vm.h"
#include "../object.h"
#include "../symbols.gen.h"
#include "../value/regexp.h"
#include "../value/class.h"

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
  return new IntValue(self->upcast<AbstractRegexpValue>()->flags());
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
  return new IntValue(REGEXP_MULTILINE);
}

/**
 * Regexp.IGNORECASE : Int
 *
 * case insensitive flag.
 */
static SharedPtr<Value> Regexp_IGNORECASE(VM* vm, Value* self) {
  return new IntValue(REGEXP_IGNORECASE);
}

/**
 * Regexp.EXPANDED : Int
 *
 * //x flag.
 */
static SharedPtr<Value> Regexp_EXPANDED(VM* vm, Value* self) {
  return new IntValue(REGEXP_EXPANDED);
}

void tora::Init_Regexp(VM* vm) {
  SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_REGEXP_CLASS);
  klass->add_method("flags", new CallbackFunction(Regexp_flags));
  klass->add_method("quotemeta", new CallbackFunction(Regexp_quotemeta));

  klass->add_method("MULTILINE", new CallbackFunction(Regexp_MULTILINE));
  klass->add_method("IGNORECASE", new CallbackFunction(Regexp_IGNORECASE));
  klass->add_method("EXPANDED", new CallbackFunction(Regexp_EXPANDED));
  vm->add_builtin_class(klass);
}
