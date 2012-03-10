#include "package.h"
#include "vm.h"
#include "value/code.h"
#include "inspector.h"
#include "peek.h"

using namespace tora;

void Package::dump(VM *vm, int indent) {
    Inspector ins(vm);

    print_indent(indent);
    printf("[dump] Package(%s):\n", vm->symbol_table->id2name(name_id).c_str());
    auto iter = data.begin();
    for (; iter!=data.end(); iter++) {
        print_indent(indent+1);
        printf("%s:\n", vm->symbol_table->id2name(iter->first).c_str());
        peek(vm, iter->second.get());
        printf("%s\n", ins.inspect(iter->second).c_str());
    }
}

void Package::add_function(ID function_name_id, const SharedPtr<Value>& code) {
    this->data[function_name_id] = code;
}

// move package to package.h/package.cc
void Package::add_method(ID function_name_id, const CallbackFunction* code) {
    SharedPtr<CodeValue> cv = new CodeValue(this->name_id, function_name_id, code);
    // printf("package!! %d::%d\n", name_id, function_name_id);
    this->data[function_name_id] = cv;
}

// assign to package variable
void Package::set_variable(ID var_name_id, const SharedPtr<Value> & v) {
    this->data[var_name_id] = v;
}

