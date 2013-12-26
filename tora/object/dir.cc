#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../object.h"
#include "../shared_ptr.h"
#include "../vm.h"
#include "../value/object.h"
#include "../value/class.h"
#include "../value/pointer.h"
#include "../value.h"
#include "../symbols.gen.h"
#include "../class_builder.h"

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
SharedValue tora::Dir_new(VM* vm, const std::string& dirname) {
  DIR* dp = opendir(dirname.c_str());
  if (dp) {
    MortalPointerValue p(dp);
    MortalObjectValue o(
      vm,
      vm->get_builtin_class(SYMBOL_DIR_CLASS).get(),
      p.get()
    );
    return o.get();
  } else {
    throw new ErrnoExceptionValue(get_errno());
  }
}

/**
 * Dir.new(Str $dirname) : Dir
 *
 * Create a new Dir class instance from $dirname.
 */
static SharedPtr<Value> dir_new(VM* vm, Value* self, Value* dirname_v) {
  return tora::Dir_new(vm, dirname_v->to_s()).get();
}

/**
 * $dir.read() : Maybe[String]
 *
 * Read a entitity from $dir.
 * If it reached end of directory, it returns undef.
 */
static SharedPtr<Value> dir_read(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_OBJECT);
  SharedPtr<Value> v = object_data(self);
  assert(v->value_type = VALUE_TYPE_POINTER);
  DIR* dp = (DIR*)get_ptr_value(v);
  assert(dp);
  // TODO: support readdir_r
  struct dirent* ent = readdir(dp);
  if (ent) {
    return new_str_value(ent->d_name);
  } else {
    return new_undef_value();
  }
}

/**
 * $dir.close() : undef
 *
 * Close directory.
 * Normally, you don't need to close directory by hand since the destructor
 *closes directory automtically.
 */
static SharedPtr<Value> dir_close(VM* vm, Value* self) {
  SharedPtr<Value> v = object_data(self);
  assert(v->value_type = VALUE_TYPE_POINTER);

  DIR* dp = static_cast<DIR*>(get_ptr_value(v));
  assert(dp);
  closedir(dp);
  return new_undef_value();
}

/**
 * Dir.mkdir(Str $name[, Int $mode]) : Undef
 *
 * Create directory named $name. Default mode value is 0777.
 *
 * This method may throws ErrnoException if it's failed.
 */
static SharedPtr<Value> dir_mkdir(VM* vm,
                                  const std::vector<SharedPtr<Value>>& args) {
  // POSIX.1-2001.
  if (args.size() != 2 && args.size() == 3) {
    throw new ArgumentExceptionValue(
        "You passed %d arguments, but usage: Dir.mkdir(Str $name[, Int $mode])",
        args.size() - 1);
  }

  int mode = args.size() == 2 ? 0777 : args[2]->to_int();
  int ret;
#ifdef _WIN32
  ret = mkdir(args[1]->to_s().c_str());
  if (ret == 0) chmod(args[1]->to_s().c_str(), mode);
#else
  ret = mkdir(args[1]->to_s().c_str(), mode);
#endif
  if (ret == 0) {
    return new_undef_value();
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
static SharedPtr<Value> dir_rmdir(VM* vm,
                                  const std::vector<SharedPtr<Value>>& args) {
  // POSIX.1-2001.
  if (args.size() != 2) {
    throw new ArgumentExceptionValue(
        "You passed %d arguments, but usage: Dir.rmdir(Str $name)",
        args.size() - 1);
  }

  if (rmdir(args[1]->to_s().c_str()) == 0) {
    return new_undef_value();
  } else {
    throw new ErrnoExceptionValue(errno);
  }
}

/**
 * $dir.DESTROY() : undef
 *
 * This is a destructor, closes directory automatically.
 */
static SharedPtr<Value> dir_DESTROY(VM* vm, Value* self) {
  dir_close(vm, self);
#ifndef NDEBUG
// self->upcast<ObjectValue>()->data().reset(new_undef_value());
#endif
  return new_undef_value();
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
static SharedPtr<Value> dir___iter__(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_OBJECT);

  MortalObjectValue v(
    vm,
    vm->get_builtin_class(SYMBOL_DIR_ITERATOR_CLASS).get(),
    self
  );
  return v.get();
}

/**
 * Dir::Iterator.__next__()
 *
 * Get a next iteration object from iterator.
 */
static SharedPtr<Value> dir_Iterator___next__(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_OBJECT);

  SharedPtr<Value> dir = object_data(self);
  if (dir->value_type != VALUE_TYPE_OBJECT) {
    return new ExceptionValue("[BUG] This is not a Directory value.: %s",
                              dir->type_str());
  }
  SharedPtr<Value> ret = dir_read(vm, dir.get());
  if (ret->value_type == VALUE_TYPE_UNDEF) {
    return new StopIterationExceptionValue();
  }
  return ret;
}

void tora::Init_Dir(VM* vm) {
  {
    ClassBuilder builder(vm, SYMBOL_DIR_CLASS);
    builder.add_method("new", new CallbackFunction(dir_new));
    builder.add_method("read", new CallbackFunction(dir_read));
    builder.add_method("close", new CallbackFunction(dir_close));
    builder.add_method("mkdir", new CallbackFunction(dir_mkdir));
    builder.add_method("rmdir", new CallbackFunction(dir_rmdir));
    builder.add_method("DESTROY", new CallbackFunction(dir_DESTROY));
    builder.add_method("__iter__", new CallbackFunction(dir___iter__));
    vm->add_builtin_class(builder.value());
  }

  {
    ClassBuilder builder(vm, SYMBOL_DIR_ITERATOR_CLASS);
    builder.add_method("__next__", new CallbackFunction(dir_Iterator___next__));
    vm->add_builtin_class(builder.value());
  }
}
