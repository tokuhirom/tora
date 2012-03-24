#include "../object.h"
#include "../vm.h"
#include "../value/file.h"
#include "../package.h"

using namespace tora;

inline static FILE *FP(Value* self) {
    assert(self->value_type == VALUE_TYPE_FILE);
    return self->upcast<FileValue>()->fp();
}

/*
 * You can pass the mode_v as NULL value.
 */
SharedPtr<Value> tora::File_open(VM *vm, Value *fname, Value *mode_v) {
    assert(fname);

    std::string mode;
    if (mode_v) {
        SharedPtr<StrValue> mode_s =  mode_v->to_s();
        mode = mode_s->str_value().c_str();
    } else {
        mode = "rb";
    }

    // TODO: check \0
    SharedPtr<FileValue> file = new FileValue();
    if (file->open(
        fname->to_s()->str_value(),
        mode
    )) {
        return file;
    } else {
        return new ExceptionValue("Cannot open file: %s: %s", fname->upcast<StrValue>()->str_value().c_str(), strerror(errno));
    }
}

/**
 * File.open(Str $fname) : FileHandle
 * File.open(Str $fname, Str $mode) : FileHandle
 */
static SharedPtr<Value> file_open_method(VM *vm, const std::vector<SharedPtr<Value>> & args) {
    if (args.size() != 2 && args.size() != 3) {
        return new ExceptionValue("Invalid argument count for open(): %zd. open() function requires 1 or 2 argument.", args.size());
    }

    return File_open(vm, args.at(1).get(), args.size()==3 ? args.at(2).get() : NULL);
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
    pkg->add_method(vm->symbol_table->get_id("open"), new CallbackFunction(file_open_method));
    pkg->add_method(vm->symbol_table->get_id("slurp"), new CallbackFunction(file_slurp));
    pkg->add_method(vm->symbol_table->get_id("close"), new CallbackFunction(file_close));
    pkg->add_method(vm->symbol_table->get_id("flush"), new CallbackFunction(file_flush));
    pkg->add_method(vm->symbol_table->get_id("fileno"), new CallbackFunction(file_fileno));
}

