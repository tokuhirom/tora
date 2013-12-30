#include "../object.h"

#include "../vm.h"
#include "../value/code.h"
#include "../value/class.h"
#include "../value/int.h"
#include "../frame.h"
#include "../callback.h"
#include "../symbols.gen.h"
#include "../class_builder.h"

using namespace tora;

/**
 * class Code
 *
 * Code class is a code object.
 */

/**
 * $code.package() : Str
 *
 * Get a declared package
 */
static SharedPtr<Value> code_package(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_CODE);
  return new_str_value(
      vm->symbol_table->id2name(code_package_id(self)));
}

/**
 * $code.name() : Str
 *
 * Returns name of code.
 */
static SharedPtr<Value> code_name(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_CODE);
  return new_str_value(
      vm->symbol_table->id2name(code_func_name_id(self)));
}

/**
 * $code.line() : Int
 *
 * Returns line number.
 */
static SharedPtr<Value> meth_code_line(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_CODE);
  MortalIntValue miv(code_lineno(self));
  return miv.get();
}

/**
 * $code.filename() : Str
 *
 * Declared file name in string.
 */
static SharedPtr<Value> meth_code_filename(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_CODE);
  MortalStrValue s(code_filename(self));
  return s.get();
}

// undocumented.
static SharedPtr<Value> code_is_closure(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_CODE);
  return vm->to_bool(code_closure_var_names(self)->size() >
                     0);
}

/**
 * $code.();
 *
 * call code object with arguments.
 */
static SharedPtr<Value> code_call(VM* vm,
                                  const std::vector<SharedPtr<Value>>& values) {
  const SharedPtr<Value> code = values[0];

  int argcnt = values.size() - 1;
  if (code_is_native(code.get())) {
    TODO();
  } else {
    for (int i = 1; i < values.size(); i++) {
      vm->stack.push_back(values[i]);
    }
    vm->function_call(argcnt, code, code);
    return new_undef_value();
  }
}

void tora::Init_Code(VM* vm) {
  ClassBuilder builder(vm, SYMBOL_CODE_CLASS);
  builder.add_method("package", std::make_shared<CallbackFunction>(code_package));
  builder.add_method("name", std::make_shared<CallbackFunction>(code_name));
  builder.add_method("line", std::make_shared<CallbackFunction>(meth_code_line));
  builder.add_method("filename", std::make_shared<CallbackFunction>(meth_code_filename));
  builder.add_method("is_closure", std::make_shared<CallbackFunction>(code_is_closure));
  builder.add_method("()", std::make_shared<CallbackFunction>(code_call));
  vm->add_builtin_class(builder.value());
}
