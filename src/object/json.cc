#include "../tora.h"
#include "../vm.h"
#include "../value/array.h"
#include "json.h"
#include "../picojson.h"
#include "array.h"

using namespace tora;

static Value* json_encode2(std::string & buf, Value *v) {
    switch (v->value_type) {
    case VALUE_TYPE_HASH: {
        buf += "{";
        auto iter = v->upcast<HashValue>()->begin();
        for (; iter!=v->upcast<HashValue>()->end(); iter++) {
            picojson::value k1(iter->first);
            buf += k1.serialize();
            buf += ":";
            Value * ret = json_encode2(buf, iter->second.get());
            if (ret) { return ret; }
        }
        buf += "}";
        break;
    }
    case VALUE_TYPE_STR: {
        picojson::value x(v->upcast<StrValue>()->str_value);
        buf += x.serialize();
        break;
    }
    case VALUE_TYPE_INT: {
        picojson::value x((double)v->upcast<IntValue>()->int_value);
        buf += x.serialize();
        break;
    }
    case VALUE_TYPE_DOUBLE: {
        picojson::value x(v->upcast<DoubleValue>()->double_value);
        buf += x.serialize();
        break;
    }
    case VALUE_TYPE_BOOL: {
        buf += v->upcast<BoolValue>()->bool_value ? "true" : "false";
        break;
    }
    case VALUE_TYPE_UNDEF: {
        buf += "null";
        break;
    }
    case VALUE_TYPE_ARRAY: {
        buf += "[";
        SharedPtr<ArrayValue> av = v->upcast<ArrayValue>();
        for (size_t i=0; i<av->size(); i++) {
            Value * ret = json_encode2(buf, av->at(i).get());
            if (ret) { return ret; }
            if (i!=av->size()-1) {
                buf += ",";
            }
        }
        buf += "]";
        break;
    }
    default:
        return new ExceptionValue("%s is not JSON serializable.", v->type_str());
    }
    return NULL;
}

/**
 * JSON.encode($value) : Str
 *
 * This method encode $value to JSON format. and return a string.
 */
static SharedPtr<Value> json_encode(VM *vm, Value *klass, Value* v) {
    std::string buf;
    Value * exception = json_encode2(buf, v);
    if (exception) {
        return exception;
    } else {
        return new StrValue(buf);
    }
}

/**
 * JSON.decode($json : Str) : Hash|Array
 *
 * Decode $json to Hash|Array.
 */
static SharedPtr<Value> json_decode(VM *vm, Value *klass, Value* v) {
    TODO();
}

void tora::Init_JSON(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("JSON");
    pkg->add_method(vm->symbol_table->get_id("encode"), new CallbackFunction(json_encode));
    pkg->add_method(vm->symbol_table->get_id("decode"), new CallbackFunction(json_decode));
}
