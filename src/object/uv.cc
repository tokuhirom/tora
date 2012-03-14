#include <uv.h>
#include "uv.h"
#include "../shared_ptr.h"
#include "../inspector.h"
#include "../value.h"
#include "../value/array.h"
#include "../package.h"
#include "../vm.h"

using namespace tora;

/*
static SharedPtr<Value> method_uv_loadavg(VM *vm, Value *self) {
    double avg[3];
    uv_loadavg(avg);
    SharedPtr<ArrayValue> av = new ArrayValue();
    for (int i=0; i<3; i++) {
        av->push(new DoubleValue(avg[i]));
    }
    return av;
}
*/

void tora::Init_UV(VM *vm) {
    SharedPtr<Package> pkg = vm->find_package("UV");
    // pkg->add_method(vm->symbol_table->get_id("loadavg"), new CallbackFunction(method_uv_loadavg));
}

