#include "nanouri.h"
#include "tora-ext.h"

extern "C" {

using namespace tora;

static SharedPtr<Value> uri_escape(VM *vm, Value *val) {
    SharedPtr<StrValue> str = val->to_s();
    return new StrValue(nu_escape_uri(str->str_value));
}

static SharedPtr<Value> uri_unescape(VM *vm, Value *val) {
    SharedPtr<StrValue> str = val->to_s();
    return new StrValue(nu_unescape_uri(str->str_value));
}

void Init_URI_Escape(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("URI::Escape");
    pkg->add_method(vm->symbol_table->get_id("uri_escape"), new CallbackFunction(uri_escape));
    pkg->add_method(vm->symbol_table->get_id("uri_unescape"), new CallbackFunction(uri_unescape));
}

}

