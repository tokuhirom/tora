#include "../vm.h"
#include "file.h"
#include "../value/file.h"

using namespace tora;

/**
 * $file.slurp()
 *
 * Read all file content and return it in string.
 */
static SharedPtr<Value> file_slurp(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_FILE);
    return self->upcast<FileValue>()->read();
}

static SharedPtr<Value> file_close(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_FILE);
    self->upcast<FileValue>()->close();
    return UndefValue::instance();
}

void tora::Init_File(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("File");
    pkg->add_method(vm->symbol_table->get_id("slurp"), new CallbackFunction(file_slurp));
    pkg->add_method(vm->symbol_table->get_id("close"), new CallbackFunction(file_close));
}

