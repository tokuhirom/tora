#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include "../object.h"
#include "../shared_ptr.h"
#include "../vm.h"
#include "../value/object.h"
#include "../value/pointer.h"
#include "../value/class.h"
#include "../value.h"
#include "../symbols.gen.h"

using namespace tora;

/**
 * class Dir
 *
 * Directory class.
 */

/**
 * Dir.new(Str $directory) : Dir
 *
 * Open directory named $directory and returns Dir object.
 */
ObjectValue* tora::Dir_new(VM *vm, StrValue *dirname) {
    DIR * dp = opendir(dirname->c_str());
    if (dp) {
        return new ObjectValue(vm, vm->get_builtin_class(SYMBOL_DIR_CLASS).get(), new PointerValue(dp));
    } else {
        throw new ErrnoExceptionValue(get_errno());
    }
}

/**
 * Dir.new(Str $dirname) : Dir
 *
 * Create a new Dir class instance from $dirname.
 */
static SharedPtr<Value> dir_new(VM * vm, Value* self, Value* dirname_v) {
    return tora::Dir_new(vm, dirname_v->to_s().get());
}

/**
 * $dir.read() : Maybe[String]
 *
 * Read a entitity from $dir.
 * If it reached end of directory, it returns undef.
 */
static SharedPtr<Value> dir_read(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> v = self->upcast<ObjectValue>()->data();
    assert(v->value_type = VALUE_TYPE_POINTER);
    DIR * dp = (DIR*)v->upcast<PointerValue>()->ptr();
    assert(dp);
    // TODO: support readdir_r
    struct dirent * ent = readdir(dp);
    if (ent) {
        return new StrValue(ent->d_name);
    } else {
        return UndefValue::instance();
    }
}

/**
 * $dir.close() : undef
 *
 * Close directory.
 * Normally, you don't need to close directory by hand since the destructor closes directory automtically.
 */
static SharedPtr<Value> dir_close(VM * vm, Value* self) {
    SharedPtr<Value> v = self->upcast<ObjectValue>()->data();
    assert(v->value_type = VALUE_TYPE_POINTER);

    DIR * dp = (DIR*)v->upcast<PointerValue>()->ptr();
    assert(dp);
    closedir(dp);
    return UndefValue::instance();
}

/**
 * Dir.mkdir(Str $name[, Int $mode]) : Undef
 *
 * Create directory named $name. Default mode value is 0777.
 *
 * This method may throws ErrnoException if it's failed.
 */
static SharedPtr<Value> dir_mkdir(VM * vm, const std::vector<SharedPtr<Value>> & args) {
    // POSIX.1-2001.
    if (args.size() != 2 && args.size() == 3) {
        throw new ArgumentExceptionValue("You passed %d arguments, but usage: Dir.mkdir(Str $name[, Int $mode])", args.size()-1);
    }

    int mode = args.size() == 2 ? 0777 : args[2]->to_int();
    int ret;
#ifdef _WIN32
	ret = mkdir(args[1]->to_s()->str_value().c_str());
    if (ret == 0)
	    chmod(args[1]->to_s()->str_value().c_str(), mode);
#else
	ret = mkdir(args[1]->to_s()->str_value().c_str(), mode);
#endif
    if (ret == 0) {
        return UndefValue::instance();
    } else {
        throw new ErrnoExceptionValue(errno);
    }
}

/**
 * Dir.rmdir(Str $name) : Undef
 *
 * Remove $name directory.
 *
 * This method may throws ErrnoException if it's failed.
 */
static SharedPtr<Value> dir_rmdir(VM * vm, const std::vector<SharedPtr<Value>> & args) {
    // POSIX.1-2001.
    if (args.size() != 2) {
        throw new ArgumentExceptionValue("You passed %d arguments, but usage: Dir.rmdir(Str $name)", args.size()-1);
    }

    if (rmdir(args[1]->to_s()->str_value().c_str()) == 0) {
        return UndefValue::instance();
    } else {
        throw new ErrnoExceptionValue(errno);
    }
}

/**
 * $dir.DESTROY() : undef
 *
 * This is a destructor, closes directory automatically.
 */
static SharedPtr<Value> dir_DESTROY(VM * vm, Value* self) {
    dir_close(vm, self);
#ifndef NDEBUG
    // self->upcast<ObjectValue>()->data().reset(UndefValue::instance());
#endif
    return UndefValue::instance();
}

/**
 * Dir.__iter__()
 *
 * Directory object can use as iterator.
 *
 * Example:
 *   for (my $e in dir('t')) { ... }
 *
 */
static SharedPtr<Value> dir___iter__(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);

    ObjectValue* v = new ObjectValue(vm, vm->get_builtin_class(SYMBOL_DIR_ITERATOR_CLASS).get(), self);
    return v;
}

/**
 * Dir::Iterator.__next__()
 *
 * Get a next iteration object from iterator.
 */
static SharedPtr<Value> dir_Iterator___next__(VM* vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);

    SharedPtr<Value> dir = self->upcast<ObjectValue>()->data();
    if (dir->value_type != VALUE_TYPE_OBJECT) { return new ExceptionValue("[BUG] This is not a Directory value.: %s", dir->type_str()); }
    SharedPtr<Value> ret = dir_read(vm, dir.get());
    if (ret->value_type == VALUE_TYPE_UNDEF) { return new StopIterationExceptionValue(); }
    return ret;
}

void tora::Init_Dir(VM *vm) {
    {
        SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_DIR_CLASS);
        klass->add_method("new",      new CallbackFunction(dir_new));
        klass->add_method("read",     new CallbackFunction(dir_read));
        klass->add_method("close",    new CallbackFunction(dir_close));
        klass->add_method("mkdir",    new CallbackFunction(dir_mkdir));
        klass->add_method("rmdir",    new CallbackFunction(dir_rmdir));
        klass->add_method("DESTROY",  new CallbackFunction(dir_DESTROY));
        klass->add_method("__iter__", new CallbackFunction(dir___iter__));
        vm->add_builtin_class(klass);
    }

    {
        SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_DIR_ITERATOR_CLASS);
        klass->add_method("__next__", new CallbackFunction(dir_Iterator___next__));
        vm->add_builtin_class(klass);
    }
}

