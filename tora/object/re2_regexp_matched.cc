#include "../object.h"

#include "../vm.h"
#include "../symbols.gen.h"
#include "../value/code.h"
#include "../symbols.gen.h"
#include "../value/object.h"
#include "../value/regexp.h"
#include "../value/class.h"
#include "../class_builder.h"

using namespace tora;

/**
 * class RE2::Regexp::Matched
 *
 * This is a regexp matched object for RE2.
 * You would get a instance of this class in String#match method.
 */

class RE2RegexpMatched {
 private:
  SharedPtr<RE2RegexpValue> re_;
  std::shared_ptr<std::vector<re2::StringPiece>> matches_;

 public:
  RE2RegexpMatched(
      RE2RegexpValue* re,
      const std::shared_ptr<std::vector<re2::StringPiece>>& matches)
      : re_(re), matches_(matches) {}
  const SharedPtr<RE2RegexpValue>& re() const { return re_; }
  const std::shared_ptr<std::vector<re2::StringPiece>>& matches() const {
    return matches_;
  }
};

static inline RE2RegexpMatched* SELF(Value* self) {
  assert(self->value_type == VALUE_TYPE_OBJECT);
  return static_cast<RE2RegexpMatched*>(
      get_ptr_value(self->upcast<ObjectValue>()->data()));
}

/**
 * $matched.regexp() : Regexp
 *
 * Get a source regular expression object.
 */
static SharedPtr<Value> RE2_Regexp_Matched_regexp(VM* vm, Value* self) {
  return SELF(self)->re();
}

/**
 * $matched.to_array() : Array[String]
 *
 * Convert $matched object to array of strings.
 */
static SharedPtr<Value> RE2_Regexp_Matched_to_array(VM* vm, Value* self) {
  MortalArrayValue ary;
  const SharedPtr<RE2RegexpValue>& re = SELF(self)->re();
  for (int i = 0; i < re->number_of_capturing_groups(); i++) {
    const re2::StringPiece& res = SELF(self)->matches()->at(i);
    MortalStrValue s(std::string(res.data(), res.length()));
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
    const SharedPtr<RE2RegexpValue>& re = SELF(self)->re();
    if (index->to_int() > re->number_of_capturing_groups()) {
      return new_undef_value();
    }
    const re2::StringPiece& res = SELF(self)->matches()->at(index->to_int());
    if (res.data()) {
      return new_str_value(std::string(res.data(), res.length()));
    } else {
      return new_undef_value();
    }
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
  delete SELF(self);
  return new_undef_value();
}

SharedPtr<Value> tora::RE2_Regexp_Matched_new(
    VM* vm, RE2RegexpValue* re,
    const std::shared_ptr<std::vector<re2::StringPiece>>& matches) {
  return new ObjectValue(
      vm, vm->get_builtin_class(SYMBOL_RE2_REGEXP_MATCHED_CLASS).get(),
      new_ptr_value(new RE2RegexpMatched(re, matches)));
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
