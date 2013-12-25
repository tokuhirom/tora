#undef __STRICT_ANSI__
#include <stdio.h>
#include <unistd.h>

#include "../object.h"
#include "../vm.h"
#include "../value/file.h"
#include "../symbols.gen.h"
#include "../value/class.h"

#ifdef _WIN32
#include <winerror.h>
#endif

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
      mode = mode_v->to_s();
    } else {
        mode = "rb";
    }

    // TODO: check \0
    SharedPtr<FileValue> file = new FileValue();
    if (file->open(
        fname->to_s(),
        mode
    )) {
        return file;
    } else {
        throw new ExceptionValue("Cannot open file: %s: %s", get_str_value(fname)->c_str(), get_strerror(get_errno()).c_str());
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
        return new ExceptionValue("Invalid argument count for open(): %ld. open() function requires 1 or 2 argument.", (long int) args.size());
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
    return new_str_value(self->upcast<FileValue>()->read());
}

/**
 * $file.close() : Undef
 *
 * Close a file.
 */
static SharedPtr<Value> file_close(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_FILE);
    self->upcast<FileValue>()->close();
    return new_undef_value();
}

/**
 * $file.write(Str $str); : Undef
 *
 * write $string to a file.
 */
static SharedPtr<Value> file_write(VM * vm, Value* self, Value *str_v) {
    assert(self->value_type == VALUE_TYPE_FILE);
    const std::string str = str_v->to_s();
    size_t ret = fwrite(str.c_str(), sizeof(char), str.size(), FP(self));
    if (ret==str.size()) {
        return new_undef_value();
    } else {
        throw new ExceptionValue("Cannot write a text for file(%ld < %ld).", (long int) ret, (long int) str.size());
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
    return new_undef_value();
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
        return new_str_value(std::string(1, (char)c));
    } else {
        if (feof(FP(self))) {
            return new_undef_value();
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
        return new_undef_value();
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

/**
 * $file.sync() : Undef
 *
 * Syncs buffers and file.
 *
 * On win32, this method is not implementede yet.
 */
static SharedPtr<Value> file_sync(VM * vm, Value* self) {
#ifdef _WIN32
    throw new ErrnoExceptionValue(ERROR_CALL_NOT_IMPLEMENTED);
#else
    // fsync: POSIX.1-2001.
    if (fsync(fileno(FP(self))) == 0) {
        return new_undef_value();
    } else {
        throw new ErrnoExceptionValue(get_errno());
    }
#endif
}

void tora::Init_File(VM* vm) {
    // isatty, __iter__, __next__, read(), readline(), readlines(), seek()
    // tell(), truncate(), write($str), fdopen
    SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_FILE_CLASS);
    klass->add_method("open",    new CallbackFunction(file_open_method));
    klass->add_method("slurp",   new CallbackFunction(file_slurp));
    klass->add_method("write",   new CallbackFunction(file_write));
    klass->add_method("print",   new CallbackFunction(file_write));
    klass->add_method("close",   new CallbackFunction(file_close));
    klass->add_method("flush",   new CallbackFunction(file_flush));
    klass->add_method("fileno",  new CallbackFunction(file_fileno));
    klass->add_method("getc",    new CallbackFunction(file_getc));
    klass->add_method("seek",    new CallbackFunction(file_seek));
    klass->add_method("tell",    new CallbackFunction(file_tell));
    klass->add_method("sync",    new CallbackFunction(file_sync));
    klass->add_constant("SEEK_SET", SEEK_SET);
    klass->add_constant("SEEK_CUR", SEEK_CUR);
    klass->add_constant("SEEK_END", SEEK_END);
    vm->add_builtin_class(klass);
}

