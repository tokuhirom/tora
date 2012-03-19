#include "../object.h"

#include "../vm.h"
#include "../value/code.h"
#include "../package.h"
#include "../frame.h"

using namespace tora;

/**
 * $code.package() : Str
 *
 * Get a declared package
 */
static SharedPtr<Value> code_package(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_CODE);
    return new StrValue(vm->symbol_table->id2name(self->upcast<CodeValue>()->package_id));
}

static SharedPtr<Value> code_name(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_CODE);
    return new StrValue(vm->symbol_table->id2name(self->upcast<CodeValue>()->func_name_id));
}

static SharedPtr<Value> code_line(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_CODE);
    return new IntValue(self->upcast<CodeValue>()->lineno);
}

static SharedPtr<Value> code_filename(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_CODE);
    return new StrValue(self->upcast<CodeValue>()->filename);
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
        return UndefValue::instance();
    }
}

void tora::Init_Code(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("Code");
    pkg->add_method(vm->symbol_table->get_id("package"), new CallbackFunction(code_package));
    pkg->add_method(vm->symbol_table->get_id("name"), new CallbackFunction(code_name));
    pkg->add_method(vm->symbol_table->get_id("line"), new CallbackFunction(code_line));
    pkg->add_method(vm->symbol_table->get_id("filename"), new CallbackFunction(code_filename));
    pkg->add_method(vm->symbol_table->get_id("()"), new CallbackFunction(code_call));
}

