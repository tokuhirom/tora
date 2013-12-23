#include "../object.h"
#include "../vm.h"
#include "../value.h"
#include "../shared_ptr.h"
#include "../value/regexp.h"
#include "../value/class.h"
#include "../value/bytes.h"
#include "../symbols.gen.h"
#include <re2/re2.h>
#include <unicode/unistr.h>

using namespace tora;

/**
 * class String
 *
 * The string class.
 */

/**
 * $string.length() : Int
 *
 * Get a String length.
 */
static SharedPtr<Value> str_length(VM *vm, Value* self) {
    return new IntValue(self->upcast<StrValue>()->length());
}

/**
 * $string.match(Regexp $pattern) : Maybe[RE2::Regexp::Matched]
 * $string.match(String $pattern) : Maybe[RE2::Regexp::Matched]
 *
 * $string match with $pattern. If it does not matched, returns undefinied value.
 * If it's matched, returns RE2::Regexp::Matched object.
 */
static SharedPtr<Value> str_match(VM *vm, Value* self_v, Value* arg1) {
    if (arg1->value_type == VALUE_TYPE_STR) {
        StrValue *self_sv = static_cast<StrValue*>(self_v);
        StrValue *pattern = static_cast<StrValue*>(arg1);
        return new BoolValue(self_sv->str_value().find(pattern->str_value()) != std::string::npos);
    } else if (arg1->value_type == VALUE_TYPE_REGEXP) {
        SharedPtr<AbstractRegexpValue> regex = arg1->upcast<AbstractRegexpValue>();
        return regex->match(vm, static_cast<StrValue*>(self_v)->str_value());
    } else {
        throw new ArgumentExceptionValue("String.match() requires string or regexp object for first argument, but you passed '%s'.", arg1->type_str());
    }
}

/**
 * $string.replace(Regexp $pattern, String $replacer) : Str
 * $string.replace(String $pattern, String $replacer) : Str
 *
 * Replace string parts by regexp or string. It returns replaced string.
 * This method does not rewrite original $string.
 */
static SharedPtr<Value> str_replace(VM *vm, Value* self, Value* arg1, Value *rewrite_v) {
    if (arg1->value_type == VALUE_TYPE_STR) {
        StrValue* pattern_sv = static_cast<StrValue*>(arg1);
        std::string pattern = RE2::QuoteMeta(pattern_sv->str_value());
        std::string ret(self->upcast<StrValue>()->str_value()); 
        RE2::GlobalReplace(&ret, pattern, rewrite_v->to_s()->str_value());
        return new StrValue(ret);
    } else if (arg1->value_type == VALUE_TYPE_REGEXP) {
        SharedPtr<AbstractRegexpValue> regex = arg1->upcast<AbstractRegexpValue>();
        int replacements;
        std::string ret = regex->replace(self->upcast<StrValue>()->str_value(), rewrite_v->to_s()->str_value(), replacements);
        return new StrValue(ret);
    } else {
        throw new ArgumentExceptionValue("String.replace requires string or regexp object for first argument, but you passed '%s'.", arg1->type_str());
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
static SharedPtr<Value> str_substr(VM *vm, const std::vector<SharedPtr<Value>>& args) {
    assert(args[0]->value_type == VALUE_TYPE_STR);
    const std::string & str = args[0]->upcast<StrValue>()->str_value();
    if (args.size()-1 == 1) { // "foobar".substr(3)
        return new StrValue(str.substr(args[1]->to_int()));
    } else if (args.size()-1 == 2) { // "foobar".substr(3,2)
        return new StrValue(str.substr(args[1]->to_int(), args[2]->to_int()));
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
        throw new ExceptionValue("First argument of String#scan must be regexp object.");
    }

    assert(self->value_type == VALUE_TYPE_STR);
    return re_v->upcast<AbstractRegexpValue>()->scan(
        vm, self->upcast<StrValue>()->str_value()
    );
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
static SharedPtr<Value> str_split(VM *vm, Value *self, const std::vector<SharedPtr<Value>> &args) {
    if (args.size() != 1 && args.size() != 2) {
        throw new ArgumentExceptionValue("Usage: $str.split(//[, $limit])");
    }
    const SharedPtr<Value> &re_v = args[0];
    if (re_v->value_type != VALUE_TYPE_REGEXP) {
        throw new ExceptionValue("First argument of String#split must be regexp object.");
    }

    int limit = 0;
    if (args.size() == 2) {
        limit = args[1]->to_int();
    }

    assert(self->value_type == VALUE_TYPE_STR);
    return re_v->upcast<AbstractRegexpValue>()->split(
        vm, self->upcast<StrValue>()->str_value(), limit
    );
}

/**
 * $string.index(Str pattern) : Int
 * $string.index(Str pattern, Int $position) : Int
 *
 */
static SharedPtr<Value> str_index(VM *vm, const std::vector<SharedPtr<Value>>& args) {
    size_t pos = 0;
    if (args.size() == 2) {
        // nop
    } else if (args.size() == 3) {
        pos = args.at(2)->to_int();
    } else {
        throw new ArgumentExceptionValue("Arguments must be 1 or 2 for Str#index");
    }

    StrValue * v = args.at(0)->upcast<StrValue>();
    size_t ret = v->str_value().find(args.at(1)->to_s()->str_value(), pos);
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
    std::string s = static_cast<StrValue*>(self)->str_value();
    std::string ret;
    std::string::iterator i = s.begin();
    std::string::iterator end = s.end();
    while (i != end) {
        ret += std::toupper((unsigned char)*i);
        ++i;
    }
    return new StrValue(ret);
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
    std::string s = static_cast<StrValue*>(self)->str_value();
    std::string ret;
    std::string::iterator i = s.begin();
    std::string::iterator end = s.end();
    while (i != end) {
        ret += std::tolower((unsigned char)*i);
        ++i;
    }
    return new StrValue(ret);
}

/**
 * $str.encode(Str $encoding='utf-8') : Bytes
 *
 * Encode a string to bytes.
 */
static SharedPtr<Value> str_encode(VM *vm, Value *self_v, const std::vector<SharedPtr<Value>> & args) {
    if (self_v->value_type != VALUE_TYPE_STR) {
        throw new ArgumentExceptionValue("This is not a string value.");
    }
    StrValue *self = static_cast<StrValue*>(self_v);
    if (args.size() == 0) { // "foobar".decode()
        return new BytesValue(self->str_value());
    } else if (args.size() == 1) {
        // convert to utf-8.
        icu::UnicodeString src(self->str_value().c_str(), "utf8");
        const char *dstcharset = args[0]->to_s()->str_value().c_str();
        int length = src.extract(0, src.length(), NULL, dstcharset);

        std::vector<char> result(length + 1);
        src.extract(0, src.length(), &result[0], dstcharset);

        return new BytesValue(std::string(result.begin(), result.end() - 1));
    } else {
        throw new ArgumentExceptionValue("Bytes#decode requires 0 or 1 arguments. but you passed %d.", args.size());
    }
}

void tora::Init_Str(VM *vm) {
    SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_STRING_CLASS);
    klass->add_method("length",  new CallbackFunction(str_length));
    klass->add_method("match",   new CallbackFunction(str_match));
    klass->add_method("replace", new CallbackFunction(str_replace));
    klass->add_method("substr",  new CallbackFunction(str_substr));
    klass->add_method("scan",    new CallbackFunction(str_scan));
    klass->add_method("split",   new CallbackFunction(str_split));
    klass->add_method("index",   new CallbackFunction(str_index));
    klass->add_method("upper",   new CallbackFunction(str_upper));
    klass->add_method("lower",   new CallbackFunction(str_lower));
    klass->add_method("encode",  new CallbackFunction(str_encode));
    vm->add_builtin_class(klass);
}

