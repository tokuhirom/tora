#include "package.h"
#include "vm.h"
#include "value/code.h"
#include "inspector.h"
#include "peek.h"

using namespace tora;

Package::~Package () {
#ifndef NDEBUG
/*
printf("BYE! PACKAGE!: %s\n", vm_->symbol_table->id2name(name_id).c_str());
*/
    for (auto iter: data) {
        if (iter.second->refcnt > 2) {
            // printf("# Possibly memory leak: %s\n", vm_->symbol_table->id2name(iter.first).c_str());
        }
    }
#endif
}

void Package::dump(VM *vm, int indent) {
    Inspector ins(vm);

    print_indent(indent);
    printf("[dump] Package(%s) super: %s:\n", vm->symbol_table->id2name(name_id).c_str(), superclass_ ? vm->symbol_table->id2name(superclass_->id()).c_str() : "(null)");
    auto iter = data.begin();
    for (; iter!=data.end(); iter++) {
        print_indent(indent+1);
        printf("%s:\n", vm->symbol_table->id2name(iter->first).c_str());
        peek(vm, iter->second.get());
        printf("%s\n", ins.inspect(iter->second).c_str());
    }
}

// pure tora
void Package::add_function(ID function_name_id, const SharedPtr<Value>& code) {
    this->data[function_name_id] = code;
}

// native
void Package::add_method(const std::string& function_name_id, const CallbackFunction* code) {
    this->add_method(vm_->symbol_table->get_id(function_name_id), code);
}
void Package::add_method(ID function_name_id, const CallbackFunction* code) {
    SharedPtr<CodeValue> cv = new CodeValue(this->name_id, function_name_id, code);
    // printf("package!! %d::%d\n", name_id, function_name_id);
    this->data[function_name_id] = cv;
}

// assign to package variable
void Package::set_variable(ID var_name_id, const SharedPtr<Value> & v) {
    this->data[var_name_id] = v;
}

/**
 * the package has a method?
 */
bool Package::has_method(ID function_name_id) {
    return this->find(function_name_id) != this->end();
}

/**
 * register constant variable.
 */
void Package::add_constant(const std::string& function_name_id, int n) {
    this->add_constant(vm_->symbol_table->get_id(function_name_id), n);
}

void Package::add_constant(ID id, int n) {
    this->add_method(id, new CallbackFunction(n));
}

