#include "inspector.h"
#include "vm.h"
#include "value.h"
#include "value/array.h"
#include "value/hash.h"
#include "value/range.h"

using namespace tora;

Inspector::Inspector(VM *vm) : vm_(vm_) {
}

std::string Inspector::inspect(const SharedPtr<Value> & v) {
    switch (v->value_type) {
    case VALUE_TYPE_UNDEF:
        return "undef";
    case VALUE_TYPE_INT:
    case VALUE_TYPE_BOOL:
    case VALUE_TYPE_STR:
    case VALUE_TYPE_DOUBLE:
        return v->to_s()->str_value;
    case VALUE_TYPE_CODE:
        return "sub { \"DUMMY\" }";
    case VALUE_TYPE_ARRAY: {
        std::string ret("[");
        SharedPtr<ArrayValue> av = v->upcast<ArrayValue>();
        bool first = true;
        for (auto iter = av->begin(); iter!=av->end(); ++iter) {
            if (!first) {
                ret += ',';
            }
            ret += this->inspect(*iter);
            first = false;
        }
        ret += "]";
        return ret;
    }
    case VALUE_TYPE_REGEXP:
        return "#<Regexp>"; // TODO
    case VALUE_TYPE_TUPLE:
        return "#<Tuple>"; // TODO
    case VALUE_TYPE_FILE:
        return "#<File>";
    case VALUE_TYPE_RANGE: {
        SharedPtr<RangeValue> rv = v->upcast<RangeValue>();
        std::ostringstream os;
        os << rv->left->to_int() << ".." << rv->right->to_int();
        return os.str();
    }
    case VALUE_TYPE_ARRAY_ITERATOR:
        return "#<Array::Iterator>";
    case VALUE_TYPE_RANGE_ITERATOR:
        return "#<Range::Iterator>";
    case VALUE_TYPE_HASH_ITERATOR:
        return "#<Hash::Iterator>";
    case VALUE_TYPE_EXCEPTION:
        return "#<Exception>";
    case VALUE_TYPE_SYMBOL:
        return "#<Symbol>";
    case VALUE_TYPE_HASH: {
        std::string ret("{");
        SharedPtr<HashValue> hv = v->upcast<HashValue>();
        bool first = true;
        for (auto iter = hv->begin(); iter!=hv->end(); ++iter) {
            if (!first) {
                ret += ',';
            }
            ret += "\"";
            ret += iter->first;
            ret += "\" => ";
            ret += this->inspect(iter->second);
            first = false;
        }
        ret += "}";
        return ret;
    }
    case VALUE_TYPE_OBJECT:
        return "#<Object>"; // TODO
    case VALUE_TYPE_POINTER:
        return "#<Pointer>"; // TODO
    }
    printf("[BUG] unknown value type");
    abort();
}

