#include "../object.h"
#include "../vm.h"
#include "../value.h"
#include "../shared_ptr.h"
#include "../value/regexp.h"
#include "../value/class.h"
#include "../value/bytes.h"
#include "../symbols.gen.h"
#include "../class_builder.h"
#include <re2/re2.h>
#include <unicode/unistr.h>

using namespace tora;

/**
 * class String
 *
 * The string class.
 */

static int utf8_length(const std::string *str_value) {
  int len = 0;
  const std::string &str = *str_value;

  for (int i = 0; i < str.size(); ++i) {
    if ((str.at(i) & 0x80) == 0) {  // 1 byte
      // printf("1 byte: %d\n", i);
      len++;
    } else {
      // printf("mutli byte! %d\n", i);
      len++;
      ++i;
      for (; i < str.size() && (((unsigned char)str.at(i)) & 0x80) == 0x80 &&
                 (str.at(i) & 0x40) == 0;
           ++i) {
        /*
        printf("mutli byte: %d %x%x\n", i,
            (unsigned char)this->str_value.at(i)&0x80,
            (unsigned char)this->str_value.at(i)&0x40
        );
        */
        // skip
      }
    }
  }
  // printf("LEN: %d\n", len);
  return len;
}

/**
 * $string.length() : Int
 *
 * Get a String length.
 */
static SharedPtr<Value> str_length(VM *vm, Value *self) {
  return new IntValue(utf8_length(get_str_value(self)));
}

/**
 * $string.match(Regexp $pattern) : Maybe[RE2::Regexp::Matched]
 * $string.match(String $pattern) : Maybe[RE2::Regexp::Matched]
 *
 * $string match with $pattern. If it does not matched, returns undefinied
 *value.
 * If it's matched, returns RE2::Regexp::Matched object.
 */
static SharedPtr<Value> str_match(VM *vm, Value *self_v, Value *arg1) {
  if (arg1->value_type == VALUE_TYPE_STR) {
    Value *pattern = arg1;
    return vm->to_bool(get_str_value(self_v)->find(*get_str_value(pattern)) !=
                       std::string::npos);
  } else if (arg1->value_type == VALUE_TYPE_REGEXP) {
    SharedPtr<AbstractRegexpValue> regex = arg1->upcast<AbstractRegexpValue>();
    return regex->match(vm, *get_str_value(self_v));
  } else {
    throw new ArgumentExceptionValue(
        "String.match() requires string or regexp object for first argument, "
        "but you passed '%s'.",
        arg1->type_str());
  }
}

/**
 * $string.replace(Regexp $pattern, String $replacer) : Str
 * $string.replace(String $pattern, String $replacer) : Str
 *
 * Replace string parts by regexp or string. It returns replaced string.
 * This method does not rewrite original $string.
 */
static SharedPtr<Value> str_replace(VM *vm, Value *self, Value *arg1,
                                    Value *rewrite_v) {
  if (arg1->value_type == VALUE_TYPE_STR) {
    std::string *pattern_sv = get_str_value(arg1);
    std::string pattern = RE2::QuoteMeta(*pattern_sv);
    std::string ret(*get_str_value(self));
    RE2::GlobalReplace(&ret, pattern, rewrite_v->to_s());
    return new_str_value(ret);
  } else if (arg1->value_type == VALUE_TYPE_REGEXP) {
    SharedPtr<AbstractRegexpValue> regex = arg1->upcast<AbstractRegexpValue>();
    int replacements;
    std::string ret =
        regex->replace(*get_str_value(self), rewrite_v->to_s(), replacements);
    return new_str_value(ret);
  } else {
    throw new ArgumentExceptionValue(
        "String.replace requires string or regexp object for first argument, "
        "but you passed '%s'.",
        arg1->type_str());
  }
}

/**
 * $string.substr(Int $start)              : Str
 * $string.substr(Int $start, Int $length) : Str
 *
 * It returns substring from $string.
 *
 * Example:
 *   "foobar".substr(3) # => "bar"
 *   "foobar".substr(3,2) # => "ba"
 *
 * Get a substring from string.
 */
static SharedPtr<Value> str_substr(VM *vm,
                                   const std::vector<SharedPtr<Value>> &args) {
  assert(args[0]->value_type == VALUE_TYPE_STR);
  const std::string *str = get_str_value(args[0]);
  if (args.size() - 1 == 1) {  // "foobar".substr(3)
    return new_str_value(str->substr(args[1]->to_int()));
  } else if (args.size() - 1 == 2) {  // "foobar".substr(3,2)
    return new_str_value(str->substr(args[1]->to_int(), args[2]->to_int()));
  } else {
    throw new ArgumentExceptionValue("String#substr requires 1 or 2 arguments");
  }
}

/**
 * $string.scan(Regexp $pattern) : Array[String]
 *
 * Scan the strings by regular expression.
 */
static SharedPtr<Value> str_scan(VM *vm, Value *self, Value *re_v) {
  if (re_v->value_type != VALUE_TYPE_REGEXP) {
    throw new ExceptionValue(
        "First argument of String#scan must be regexp object.");
  }

  assert(self->value_type == VALUE_TYPE_STR);
  return re_v->upcast<AbstractRegexpValue>()->scan(vm, *get_str_value(self));
}

/**
 * $string.split(Regexp $pattern[, Int $limit]) : Array[String]
 *
 * Examples:
 *   "foo".split(//) # => qw(f o o)
 *   "a\nb\nc".split(/\n/) # => qw(a b c)
 *
 * Split a string by regexp.
 */
static SharedPtr<Value> str_split(VM *vm, Value *self,
                                  const std::vector<SharedPtr<Value>> &args) {
  if (args.size() != 1 && args.size() != 2) {
    throw new ArgumentExceptionValue("Usage: $str.split(//[, $limit])");
  }
  const SharedPtr<Value> &re_v = args[0];
  if (re_v->value_type != VALUE_TYPE_REGEXP) {
    throw new ExceptionValue(
        "First argument of String#split must be regexp object.");
  }

  int limit = 0;
  if (args.size() == 2) {
    limit = args[1]->to_int();
  }

  assert(self->value_type == VALUE_TYPE_STR);
  return re_v->upcast<AbstractRegexpValue>()->split(vm, *get_str_value(self),
                                                    limit);
}

/**
 * $string.index(Str pattern) : Int
 * $string.index(Str pattern, Int $position) : Int
 *
 */
static SharedPtr<Value> str_index(VM *vm,
                                  const std::vector<SharedPtr<Value>> &args) {
  size_t pos = 0;
  if (args.size() == 2) {
    // nop
  } else if (args.size() == 3) {
    pos = args.at(2)->to_int();
  } else {
    throw new ArgumentExceptionValue("Arguments must be 1 or 2 for Str#index");
  }

  Value *v = args.at(0).get();
  size_t ret = get_str_value(v)->find(args.at(1)->to_s(), pos);
  return new IntValue(ret == std::string::npos ? -1 : ret);
}

/**
 * $string.upper() : Str
 *
 * Return a copy of s, but with upper case letters converted to upper case.
 */
static SharedPtr<Value> str_upper(VM *vm, Value *self) {
  if (self->value_type != VALUE_TYPE_STR) {
    throw new ExceptionValue("This is a method for string object.");
  }
  std::string s(*get_str_value(self));
  std::string ret;
  std::string::iterator i = s.begin();
  std::string::iterator end = s.end();
  while (i != end) {
    ret += std::toupper((unsigned char)*i);
    ++i;
  }
  return new_str_value(ret);
}

/**
 * $string.lower() : Str
 *
 * Return a copy of s, but with upper case letters converted to lower case.
 */
static SharedPtr<Value> str_lower(VM *vm, Value *self) {
  if (self->value_type != VALUE_TYPE_STR) {
    throw new ExceptionValue("This is a method for string object.");
  }
  std::string s = *get_str_value(self);
  std::string ret;
  std::string::iterator i = s.begin();
  std::string::iterator end = s.end();
  while (i != end) {
    ret += std::tolower((unsigned char)*i);
    ++i;
  }
  return new_str_value(ret);
}

/**
 * $str.encode(Str $encoding='utf-8') : Bytes
 *
 * Encode a string to bytes.
 */
static SharedPtr<Value> str_encode(VM *vm, Value *self_v,
                                   const std::vector<SharedPtr<Value>> &args) {
  if (self_v->value_type != VALUE_TYPE_STR) {
    throw new ArgumentExceptionValue("This is not a string value.");
  }
  if (args.size() == 0) {  // "foobar".decode()
    return new BytesValue(*get_str_value(self_v));
  } else if (args.size() == 1) {
    // convert to utf-8.
    icu::UnicodeString src(get_str_value(self_v)->c_str(), "utf8");
    std::string dstcharset = args[0]->to_s();
    int length = src.extract(0, src.length(), NULL, dstcharset.c_str());

    std::vector<char> result(length + 1);
    src.extract(0, src.length(), &result[0], dstcharset.c_str());

    return new BytesValue(std::string(result.begin(), result.end() - 1));
  } else {
    throw new ArgumentExceptionValue(
        "Bytes#decode requires 0 or 1 arguments. but you passed %d.",
        args.size());
  }
}

void tora::Init_Str(VM *vm) {
  ClassBuilder builder(vm, SYMBOL_STRING_CLASS);
  builder.add_method("length", new CallbackFunction(str_length));
  builder.add_method("match", new CallbackFunction(str_match));
  builder.add_method("replace", new CallbackFunction(str_replace));
  builder.add_method("substr", new CallbackFunction(str_substr));
  builder.add_method("scan", new CallbackFunction(str_scan));
  builder.add_method("split", new CallbackFunction(str_split));
  builder.add_method("index", new CallbackFunction(str_index));
  builder.add_method("upper", new CallbackFunction(str_upper));
  builder.add_method("lower", new CallbackFunction(str_lower));
  builder.add_method("encode", new CallbackFunction(str_encode));
  vm->add_builtin_class(builder.value());
}
