#include "../object.h"
#include "../vm.h"
#include "../value.h"
#include "../shared_ptr.h"
#include "../value/regexp.h"
#include "../package.h"
#include <re2/re2.h>

using namespace tora;

static SharedPtr<Value> str_length(VM *vm, Value* self) {
    return new IntValue(self->upcast<StrValue>()->length());
}

/**
 * 'foo'.match(/oo/) : boolean
 * 'foo'.match('oo') : boolean
 *
 * match.
 */
static SharedPtr<Value> str_match(VM *vm, Value* self_v, Value* arg1) {
    if (arg1->value_type == VALUE_TYPE_STR) {
        StrValue *self_sv = static_cast<StrValue*>(self_v);
        StrValue *pattern = static_cast<StrValue*>(arg1);
        return new BoolValue(self_sv->str_value().find(pattern->str_value()) != std::string::npos);
    } else if (arg1->value_type == VALUE_TYPE_REGEXP) {
        SharedPtr<AbstractRegexpValue> regex = arg1->upcast<AbstractRegexpValue>();
        return new BoolValue(regex->match(static_cast<StrValue*>(self_v)->str_value()));
    } else {
        throw new ArgumentExceptionValue("String.match() requires string or regexp object for first argument, but you passed '%s'.", arg1->type_str());
    }
}

/**
 * "foo".replace(/o/g, 'p') : Str
 * "foo".replace(/o/,  'p') : Str
 * "foo".replace('o',  'p') : Str
 *
 * Replace string parts.
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

void tora::Init_Str(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("String");
    pkg->add_method(vm->symbol_table->get_id("length"), new CallbackFunction(str_length));
    pkg->add_method(vm->symbol_table->get_id("match"), new CallbackFunction(str_match));
    pkg->add_method(vm->symbol_table->get_id("replace"), new CallbackFunction(str_replace));
}

