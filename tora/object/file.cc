#undef __STRICT_ANSI__
#include <stdio.h>

#include "../object.h"
#include "../vm.h"
#include "../value/file.h"
#include "../package.h"

using namespace tora;

/**
 * class File
 *
 * The file class.
 */

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
        return new ExceptionValue("Cannot open file: %s: %s", fname->upcast<StrValue>()->str_value().c_str(), get_strerror(get_errno()).c_str());
    }
}

/**
 * File.open(Str $fname) : FileHandle
 * File.open(Str $fname, Str $mode) : FileHandle
 *
 * Open the file named $fname by $mode.
 *
 * Str $mode: file opening mode. You can specify the option as following:
 *   "w": writing mode
 *   "r": reading mode
 *   "a": appending mode
 *
 * File.open passes $mode to fopen(3).
 */
static SharedPtr<Value> file_open_method(VM *vm, const std::vector<SharedPtr<Value>> & args) {
    if (args.size() != 2 && args.size() != 3) {
        return new ExceptionValue("Invalid argument count for open(): %zd. open() function requires 1 or 2 argument.", args.size());
    }

    return File_open(vm, args.at(1).get(), args.size()==3 ? args.at(2).get() : NULL);
}

/**
 * $file.slurp() : String
 *
 * Read all file content and return it in string.
 */
static SharedPtr<Value> file_slurp(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_FILE);
    return self->upcast<FileValue>()->read();
}

/**
 * $file.close() : Undef
 *
 * Close a file.
 */
static SharedPtr<Value> file_close(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_FILE);
    self->upcast<FileValue>()->close();
    return UndefValue::instance();
}

/**
 * $file.write(Str $str); : Undef
 *
 * write $string to a file.
 */
static SharedPtr<Value> file_write(VM * vm, Value* self, Value *str_v) {
    assert(self->value_type == VALUE_TYPE_FILE);
    const std::string & str = str_v->to_s()->str_value();
    size_t ret = fwrite(str.c_str(), sizeof(char), str.size(), FP(self));
    if (ret==str.size()) {
        return UndefValue::instance();
    } else {
        throw new ExceptionValue("Cannot write a text for file(%zd < %zd).", ret, str.size());
    }
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

/**
 * $file.getc() : Maybe[Str]
 *
 * Get a character from $file.
 * If EOF reached, returns undef.
 */
static SharedPtr<Value> file_getc(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_FILE);
    int c = fgetc(FP(self));
    if (c>=0) {
        return new StrValue(std::string(1, (char)c));
    } else {
        if (feof(FP(self))) {
            return UndefValue::instance();
        } else {
            throw new ErrnoExceptionValue(get_errno());
        }
    }
}

/**
 * $file.seek(Int offset, Int $whence) : Undef
 *
 * Seek a file pointer position from $file.
 * $whence must be one of File.SEEK_CUR, File.SEEK_END, File.SEEK_SET.
 *
 * maybe throw ErrnoException.
 */
static SharedPtr<Value> file_seek(VM * vm, Value* self, Value *offset, Value *whence) {
    if (fseek(FP(self), offset->to_int(), whence->to_int()) == 0) {
        return UndefValue::instance();
    } else {
        throw new ErrnoExceptionValue(get_errno());
    }
}

/**
 * $file.tell() : Int
 *
 * Get a file pointer position from $file.
 *
 * maybe throw ErrnoException.
 */
static SharedPtr<Value> file_tell(VM * vm, Value* self) {
    long ret = ftell(FP(self));
    if (ret >= 0) {
        return new IntValue(ret);
    } else {
        throw new ErrnoExceptionValue(get_errno());
    }
}

void tora::Init_File(VM* vm) {
    // isatty, __iter__, __next__, read(), readline(), readlines(), seek()
    // tell(), truncate(), write($str), fdopen
    SharedPtr<Package> pkg = vm->find_package("File");
    pkg->add_method("open",   new CallbackFunction(file_open_method));
    pkg->add_method("slurp",  new CallbackFunction(file_slurp));
    pkg->add_method("write",  new CallbackFunction(file_write));
    pkg->add_method("close",  new CallbackFunction(file_close));
    pkg->add_method("flush",  new CallbackFunction(file_flush));
    pkg->add_method("fileno", new CallbackFunction(file_fileno));
    pkg->add_method("getc",   new CallbackFunction(file_getc));
    pkg->add_method("seek",   new CallbackFunction(file_seek));
    pkg->add_method("tell",   new CallbackFunction(file_tell));
    pkg->add_constant("SEEK_SET", SEEK_SET);
    pkg->add_constant("SEEK_CUR", SEEK_CUR);
    pkg->add_constant("SEEK_END", SEEK_END);
}

