#include "../object.h"

#include "../vm.h"
#include "../value/code.h"
#include "../value/class.h"
#include "../frame.h"
#include "../callback.h"
#include "../symbols.gen.h"

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
static SharedPtr<Value> code_package(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_CODE);
    return new_str_value(vm->symbol_table->id2name(self->upcast<CodeValue>()->package_id()));
}

/**
 * $code.name() : Str
 *
 * Returns name of code.
 */
static SharedPtr<Value> code_name(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_CODE);
    return new_str_value(vm->symbol_table->id2name(self->upcast<CodeValue>()->func_name_id()));
}

/**
 * $code.line() : Int
 *
 * Returns line number.
 */
static SharedPtr<Value> code_line(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_CODE);
    return new IntValue(self->upcast<CodeValue>()->lineno());
}

/**
 * $code.filename() : Str
 *
 * Declared file name in string.
 */
static SharedPtr<Value> code_filename(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_CODE);
    return new_str_value(self->upcast<CodeValue>()->filename());
}

// undocumented.
static SharedPtr<Value> code_is_closure(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_CODE);
    return vm->to_bool(self->upcast<CodeValue>()->closure_var_names()->size() > 0);
}

/**
 * $code.();
 *
 * call code object with arguments.
 */
static SharedPtr<Value> code_call(VM * vm, const std::vector<SharedPtr<Value>> &values) {
    const SharedPtr<CodeValue> code = values[0]->upcast<CodeValue>();

    int argcnt = values.size()-1;
    if (code->is_native()) {
        TODO();
    } else {
        for (int i=1; i<values.size(); i++) {
            vm->stack.push_back(values[i]);
        }
        vm->function_call(argcnt, code, code);
        return new_undef_value();
    }
}

void tora::Init_Code(VM* vm) {
    SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_CODE_CLASS);
    klass->add_method("package",    new CallbackFunction(code_package));
    klass->add_method("name",       new CallbackFunction(code_name));
    klass->add_method("line",       new CallbackFunction(code_line));
    klass->add_method("filename",   new CallbackFunction(code_filename));
    klass->add_method("is_closure", new CallbackFunction(code_is_closure));
    klass->add_method("()",         new CallbackFunction(code_call));
    vm->add_builtin_class(klass);
}

