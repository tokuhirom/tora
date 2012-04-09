#include "nanoircparser.hpp"
#include "tora.h"
#include <vm.h>
#include <value.h>
#include <value/class.h>
#include <value/tuple.h>

using namespace tora;

static SharedPtr<Value> parse_irc(VM *vm, Value *val) {
    NanoIRCParser parser;
    NanoIRCParser::Message msg;
    int ret = parser.parse(val->to_s()->str_value(), msg);
    SharedPtr<TupleValue> t = new TupleValue();
    SharedPtr<ArrayValue> av = new ArrayValue();
    for (auto iter=msg.params.begin(); iter!=msg.params.end(); iter++) {
        av->push_back(new StrValue(*iter));
    }
    t->push_back(av);
    t->push_back(new StrValue(msg.command));
    t->push_back(new StrValue(msg.prefix));
    t->push_back(new IntValue(ret));
    return t;
}

extern "C" {

TORA_EXPORT
void Init_IRC(VM *vm) {
    vm->add_function("parse_irc",   new CallbackFunction(parse_irc));
}

}
