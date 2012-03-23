#include "tora.h"
#include <vm.h>
#include <value/object.h>
#include <value/pointer.h>
#include <package.h>
#include <shared_ptr.h>

#include <curl/curl.h>

using namespace tora;

void Init_Curl_Constants(VM* vm);

inline CURL *SELF(Value *self) {
    CURL * curl = static_cast<void*>(self->upcast<ObjectValue>()->data()->upcast<PointerValue>()->ptr());
    return curl;
}

static SharedPtr<Value> Curl_Easy_new(VM *vm, Value *klass) {
    return new ObjectValue(vm, vm->symbol_table->get_id("Curl::Easy"), new PointerValue(curl_easy_init()));
}

static SharedPtr<Value> Curl_Easy_DESTROY(VM *vm, Value *self) {
    curl_easy_cleanup(SELF(self));
}

extern "C" {

void Init_Curl(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("Curl::Easy");
    pkg->add_method(vm->symbol_table->get_id("new"), new CallbackFunction(Curl_Easy_new));
    pkg->add_method(vm->symbol_table->get_id("DESTROY"), new CallbackFunction(Curl_Easy_DESTROY));
    pkg->add_constant("CURLOPT_URL", CURLOPT_URL);
}

}
