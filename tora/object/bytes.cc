#include <stdlib.h>

#include "../vm.h"
#include "../object.h"
#include "../value/bytes.h"
#include "../value/exception.h"
#include "../value.h"
#include "../value/class.h"
#include "../value/int.h"
#include "../symbols.gen.h"
#include "../class_builder.h"
#include <unicode/unistr.h>

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
static SharedPtr<Value> meth_bytes_length(VM *vm, Value *self) {
  assert(self->value_type == VALUE_TYPE_BYTES);
  MortalIntValue i(bytes_length(self));
  return i.get();
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
static SharedPtr<Value> Bytes_substr(
    VM *vm, const std::vector<SharedPtr<Value>> &args) {
  assert(args[0]->value_type == VALUE_TYPE_BYTES);
  std::string *str = get_bytes_value(args[0].get());
  if (args.size() - 1 == 1) {  // "foobar".substr(3)
    MortalBytesValue b(str->substr(args[1]->to_int()));
    return b.get();
  } else if (args.size() - 1 == 2) {  // "foobar".substr(3,2)
    MortalBytesValue b(str->substr(args[1]->to_int(), args[2]->to_int()));
    return b.get();
  } else {
    throw new ArgumentExceptionValue("Bytes#substr requires 1 or 2 arguments");
  }
}

std::string eucjpToUtf8(const std::string &value) {
  icu::UnicodeString src(value.c_str(), "euc-jp");
  int length = src.extract(0, src.length(), NULL, "utf-8");

  std::vector<char> result(length + 1);
  src.extract(0, src.length(), &result[0], "utf-16");

  return std::string(result.begin(), result.end() - 1);
}

/**
 * $bytes.decode(Str $type="utf-8") : Undef
 */
static SharedPtr<Value> Bytes_decode(
    VM *vm, Value *self, const std::vector<SharedPtr<Value>> &args) {
  if (self->value_type != VALUE_TYPE_BYTES) {
    throw new ArgumentExceptionValue("This is not a bytes value.");
  }
  if (args.size() == 0) {  // "foobar".decode()
    MortalStrValue s(*get_bytes_value(self));
    return s.get();
  } else if (args.size() == 1) {
    // convert to utf-8.
    std::string srccharset = args[0]->to_s();
    icu::UnicodeString src(get_bytes_value(self)->c_str(),
                           get_bytes_value(self)->length(), srccharset.c_str());
    std::string buf;
    src.toUTF8String(buf);
    MortalStrValue s(buf);
    return s.get();
  } else {
    throw new ArgumentExceptionValue(
        "Bytes#decode requires 0 or 1 arguments. but you passed %d.",
        args.size());
  }
}

void tora::Init_Bytes(VM *vm) {
  ClassBuilder builder(vm, SYMBOL_BYTES_CLASS);
  builder.add_method("length", std::make_shared<CallbackFunction>(meth_bytes_length));
  builder.add_method("substr", std::make_shared<CallbackFunction>(Bytes_substr));
  builder.add_method("decode", std::make_shared<CallbackFunction>(Bytes_decode));
  vm->add_builtin_class(builder.value());
}
