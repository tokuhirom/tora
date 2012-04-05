#include <stdlib.h>

#include "../vm.h"
#include "../object.h"
#include "../package.h"
#include "../value/bytes.h"
#include "../value.h"
#include "../value/str.h"
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

std::string eucjpToUtf8(const std::string& value)
{
    icu::UnicodeString src(value.c_str(), "euc-jp");
    int length = src.extract(0, src.length(), NULL, "utf-8");

    std::vector<char> result(length + 1);
    src.extract(0, src.length(), &result[0], "utf-16");

    return std::string(result.begin(), result.end() - 1);
}


/**
 * $bytes.decode(Str $type="utf-8") : Undef
 */
static SharedPtr<Value> Bytes_decode(VM *vm, const std::vector<SharedPtr<Value>>& args) {
    if (args.size()==0) {
        throw new ArgumentExceptionValue("Bytes#decode requires 0 or 1 arguments. but you passed %d.", args.size()-1);
    }

    if (args[0]->value_type != VALUE_TYPE_BYTES) {
        throw new ArgumentExceptionValue("This is not a bytes value.");
    }
    BytesValue *self = args.at(0)->upcast<BytesValue>();
    if (args.size() == 1) { // "foobar".decode()
        return new StrValue(self->str_value());
    } else if (args.size() == 2) {
        // convert to utf-8.
        const char *srccharset = args[1]->to_s()->str_value().c_str();
        icu::UnicodeString src(self->str_value().c_str(), self->str_value().length(), srccharset);
        std::string buf;
        src.toUTF8String(buf);
        return new StrValue(buf);
    } else {
        throw new ArgumentExceptionValue("Bytes#decode requires 0 or 1 arguments. but you passed %d.", args.size()-1);
    }
}

void tora::Init_Bytes(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("Bytes");
    pkg->add_method(vm->symbol_table->get_id("length"), new CallbackFunction(bytes_length));
    pkg->add_method("substr", new CallbackFunction(Bytes_substr));
    pkg->add_method("decode", new CallbackFunction(Bytes_decode));
}

