#include <stdlib.h>

#include "../vm.h"
#include "../object.h"
#include "../value/bytes.h"
#include "../value.h"
#include "../value/class.h"
#include "../symbols.gen.h"
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
static SharedPtr<Value> bytes_length(VM *vm, Value *self) {
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
static SharedPtr<Value> Bytes_substr(
    VM *vm, const std::vector<SharedPtr<Value>> &args) {
  assert(args[0]->value_type == VALUE_TYPE_BYTES);
  const std::string &str = args[0]->upcast<BytesValue>()->str_value();
  if (args.size() - 1 == 1) {  // "foobar".substr(3)
    return new BytesValue(str.substr(args[1]->to_int()));
  } else if (args.size() - 1 == 2) {  // "foobar".substr(3,2)
    return new BytesValue(str.substr(args[1]->to_int(), args[2]->to_int()));
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
    VM *vm, Value *self_v, const std::vector<SharedPtr<Value>> &args) {
  if (self_v->value_type != VALUE_TYPE_BYTES) {
    throw new ArgumentExceptionValue("This is not a bytes value.");
  }
  BytesValue *self = static_cast<BytesValue *>(self_v);
  if (args.size() == 0) {  // "foobar".decode()
    return new_str_value(self->str_value());
  } else if (args.size() == 1) {
    // convert to utf-8.
    std::string srccharset = args[0]->to_s();
    icu::UnicodeString src(self->str_value().c_str(),
                           self->str_value().length(), srccharset.c_str());
    std::string buf;
    src.toUTF8String(buf);
    return new_str_value(buf);
  } else {
    throw new ArgumentExceptionValue(
        "Bytes#decode requires 0 or 1 arguments. but you passed %d.",
        args.size());
  }
}

void tora::Init_Bytes(VM *vm) {
  SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_BYTES_CLASS);
  klass->add_method(vm->symbol_table->get_id("length"),
                    new CallbackFunction(bytes_length));
  klass->add_method("substr", new CallbackFunction(Bytes_substr));
  klass->add_method("decode", new CallbackFunction(Bytes_decode));
  vm->add_builtin_class(klass);
}
