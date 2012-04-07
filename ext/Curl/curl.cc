#include "tora.h"
#include <vm.h>
#include <value/object.h>
#include <value/pointer.h>
#include <value/class.h>
#include <shared_ptr.h>

#include <curl/curl.h>

using namespace tora;

void Init_Curl_Constants(VM* vm);

inline CURL *SELF(Value *self) {
    CURL * curl = static_cast<void*>(self->upcast<ObjectValue>()->data()->upcast<PointerValue>()->ptr());
    return curl;
}

static SharedPtr<Value> Curl_Easy_new(VM *vm, Value *klass) {
    assert(klass->value_type == VALUE_TYPE_CLASS);
    return new ObjectValue(vm, klass->upcast<ClassValue>(), new PointerValue(curl_easy_init()));
}

static SharedPtr<Value> Curl_Easy_DESTROY(VM *vm, Value *self) {
    curl_easy_cleanup(SELF(self));
    return UndefValue::instance();
}

TORA_EXPORT
void Init_Curl(VM* vm) {
    SharedPtr<ClassValue> klass = new ClassValue(vm, "Curl::Easy");
    klass->add_method(vm->symbol_table->get_id("new"), new CallbackFunction(Curl_Easy_new));
    klass->add_method(vm->symbol_table->get_id("DESTROY"), new CallbackFunction(Curl_Easy_DESTROY));
    klass->add_constant("CURLOPT_URL", CURLOPT_URL);
    vm->add_class(klass);
}

