#include "../vm.h"
#include "file.h"
#include "../value/file.h"

using namespace tora;

inline static FILE *FP(Value* self) {
    assert(self->value_type == VALUE_TYPE_FILE);
    return self->upcast<FileValue>()->fp;
}

/**
 * $file.slurp()
 *
 * Read all file content and return it in string.
 */
static SharedPtr<Value> file_slurp(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_FILE);
    return self->upcast<FileValue>()->read();
}

/**
 * $file.close();
 *
 * Close a file.
 */
static SharedPtr<Value> file_close(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_FILE);
    self->upcast<FileValue>()->close();
    return UndefValue::instance();
}

/**
 * $file.flush();
 *
 * flush a writing buffer in stdio.
 */
static SharedPtr<Value> file_flush(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_FILE);
    fflush(FP(self));
    return UndefValue::instance();
}

/**
 * $file.fileno();
 *
 * Get a file number in file handler.
 */
static SharedPtr<Value> file_fileno(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_FILE);
    return new IntValue(fileno(FP(self)));
}

void tora::Init_File(VM* vm) {
    // isatty, __iter__, __next__, read(), readline(), readlines(), seek()
    // tell(), truncate(), write($str), fdopen
    SharedPtr<Package> pkg = vm->find_package("File");
    pkg->add_method(vm->symbol_table->get_id("slurp"), new CallbackFunction(file_slurp));
    pkg->add_method(vm->symbol_table->get_id("close"), new CallbackFunction(file_close));
    pkg->add_method(vm->symbol_table->get_id("flush"), new CallbackFunction(file_flush));
    pkg->add_method(vm->symbol_table->get_id("fileno"), new CallbackFunction(file_fileno));
}

