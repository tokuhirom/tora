#include "../object.h"

#include "../vm.h"
#include "../symbols.gen.h"
#include "../value/code.h"
#include "../symbols.gen.h"
#include "../value/object.h"
#include "../value/regexp.h"
#include "../value/class.h"
#include "../value/pointer.h"
#include "../class_builder.h"

using namespace tora;

/**
 * class RE2::Regexp::Matched
 *
 * This is a regexp matched object for RE2.
 * You would get a instance of this class in String#match method.
 */

/**
 * $matched.regexp() : Regexp
 *
 * Get a source regular expression object.
 */
static SharedPtr<Value> RE2_Regexp_Matched_regexp(VM* vm, Value* self) {
  return regexp_matched_regexp(self).get();
}

/**
 * $matched.to_array() : Array[String]
 *
 * Convert $matched object to array of strings.
 */
static SharedPtr<Value> RE2_Regexp_Matched_to_array(VM* vm, Value* self) {
  MortalArrayValue ary;
  SharedValue re = regexp_matched_regexp(self);
  for (int i = 0; i < regexp_number_of_capturing_groups(re.get()); i++) {
    SharedValue s = regexp_matches_get_item(self, i);
    array_push_back(ary.get(), s.get());
  }
  return ary.get();
}

/**
 * $matched.__getitem__(Int $i)    : Maybe[String]
 * $matched.__getitem__(Str $name) : Maybe[String]
 *
 * Get a matched string piece by index $i.
 *
 * $matched.__getitem__(Str $name) returns named capture string, but it is not
 *implemented yet.
 */
static SharedPtr<Value> RE2_Regexp_Matched_getitem(VM* vm, Value* self,
                                                   Value* index) {
  if (index->value_type == VALUE_TYPE_INT) {
    SharedValue re = regexp_matched_regexp(self);
    if (index->to_int() > regexp_number_of_capturing_groups(re.get())) {
      return new_undef_value();
    }
    SharedValue res = regexp_matches_get_item(self, index->to_int());
    return res.get();
    // TODO: check out of range
  } else if (index->value_type == VALUE_TYPE_STR) {
    TODO();
  } else {
    throw new ExceptionValue(
        "RE2::Regexp::Matched[index], index should be string or integer but "
        "you passed %s",
        (long int)index->type_str());
  }
}

static SharedPtr<Value> RE2_Regexp_Matched_DESTROY(VM* vm, Value* self) {
  regexp_matched_free(self);
  MortalUndefValue u;
  return u.get();
}

void tora::Init_RE2_Regexp_Matched(VM* vm) {
  ClassBuilder builder(vm, SYMBOL_RE2_REGEXP_MATCHED_CLASS);
  builder.add_method("regexp", new CallbackFunction(RE2_Regexp_Matched_regexp));
  builder.add_method("to_array",
                    new CallbackFunction(RE2_Regexp_Matched_to_array));
  builder.add_method("__getitem__",
                    new CallbackFunction(RE2_Regexp_Matched_getitem));
  builder.add_method("DESTROY",
                    new CallbackFunction(RE2_Regexp_Matched_DESTROY));
  vm->add_builtin_class(builder.value());
}
