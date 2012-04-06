#include "nanouri.h"
#include "tora.h"
#include <vm.h>
#include <value.h>
#include <value/class.h>

using namespace tora;

static SharedPtr<Value> uri_escape(VM *vm, Value *val) {
    SharedPtr<StrValue> str = val->to_s();
    return new StrValue(nu_escape_uri(str->str_value()));
}

static SharedPtr<Value> uri_unescape(VM *vm, Value *val) {
    SharedPtr<StrValue> str = val->to_s();
    return new StrValue(nu_unescape_uri(str->str_value()));
}

extern "C" {

TORA_EXPORT
void Init_URI_Escape(VM *vm) {
    SharedPtr<ClassValue> klass = new ClassValue(vm, "URI::Escape");
    klass->add_method(vm->symbol_table->get_id("uri_escape"), new CallbackFunction(uri_escape));
    klass->add_method(vm->symbol_table->get_id("uri_unescape"), new CallbackFunction(uri_unescape));
    vm->add_class(klass);
}

}

