#include "../object.h"

#include "../vm.h"
#include "../symbols.gen.h"
#include "../value/code.h"
#include "../package.h"
#include "../value/pointer.h"
#include "../value/object.h"
#include "../value/regexp.h"

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
    boost::shared_array<re2::StringPiece> matches_;
public:
    RE2RegexpMatched(RE2RegexpValue* re, const boost::shared_array<re2::StringPiece> & matches) : re_(re), matches_(matches) {
    }
    const SharedPtr<RE2RegexpValue>& re() const {
        return re_;
    }
    boost::shared_array<re2::StringPiece> matches() const {
        return matches_;
    }
};

static inline RE2RegexpMatched * SELF(Value *self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    return static_cast<RE2RegexpMatched*>(self->upcast<ObjectValue>()->data()->upcast<PointerValue>()->ptr());
}

/**
 * $matched.regexp() : Regexp
 *
 * Get a source regular expression object.
 */
static SharedPtr<Value> RE2_Regexp_Matched_regexp(VM * vm, Value* self) {
    return SELF(self)->re();
}

/**
 * $matched.to_array() : Array[String]
 *
 * Convert $matched object to array of strings.
 */
static SharedPtr<Value> RE2_Regexp_Matched_to_array(VM * vm, Value* self) {
    SharedPtr<ArrayValue> ary = new ArrayValue();
    const SharedPtr<RE2RegexpValue>& re = SELF(self)->re();
    for (int i=0; i<re->number_of_capturing_groups(); i++) {
        re2::StringPiece* res = &(SELF(self)->matches()[i]);
        ary->push_back(new StrValue(std::string(res->data(), res->length())));
    }
    return ary;
}

/**
 * $matched.__getitem__(Int $i)    : Maybe[String]
 * $matched.__getitem__(Str $name) : Maybe[String]
 *
 * Get a matched string piece by index $i.
 *
 * $matched.__getitem__(Str $name) returns named capture string, but it is not implemented yet.
 */
static SharedPtr<Value> RE2_Regexp_Matched_getitem(VM * vm, Value* self, Value *index) {
    if (index->value_type == VALUE_TYPE_INT) {
        const SharedPtr<RE2RegexpValue>& re = SELF(self)->re();
        if (index->to_int() > re->number_of_capturing_groups()) {
            return UndefValue::instance();
        }
        re2::StringPiece* res = &(SELF(self)->matches()[index->to_int()]);
        if(res->data()) {
            return new StrValue(std::string(res->data(), res->length()));
        } else {
            return UndefValue::instance();
        }
        // TODO: check out of range
    } else if (index->value_type == VALUE_TYPE_STR) {
        TODO();
    } else {
        throw new ExceptionValue("RE2::Regexp::Matched[index], index should be string or integer but you passed %s", index->type_str());
    }
}

static SharedPtr<Value> RE2_Regexp_Matched_DESTROY(VM * vm, Value* self) {
    delete SELF(self);
    return UndefValue::instance();
}

SharedPtr<Value> tora::RE2_Regexp_Matched_new(VM *vm, RE2RegexpValue* re, const boost::shared_array<re2::StringPiece> & matches) {
    return new ObjectValue(vm, SYMBOL_RE2_REGEXP_MATCHED_CLASS, new PointerValue(new RE2RegexpMatched(re, matches)));
}

void tora::Init_RE2_Regexp_Matched(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package(SYMBOL_RE2_REGEXP_MATCHED_CLASS);
    pkg->add_method("regexp",            new CallbackFunction(RE2_Regexp_Matched_regexp));
    pkg->add_method("to_array",          new CallbackFunction(RE2_Regexp_Matched_to_array));
    pkg->add_method(SYMBOL___GET_ITEM__, new CallbackFunction(RE2_Regexp_Matched_getitem));
    pkg->add_method("DESTROY",           new CallbackFunction(RE2_Regexp_Matched_DESTROY));
}

