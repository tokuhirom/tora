#include "vm.h"
// #include "package.h"

using namespace tora;

void Package::dump(SharedPtr<SymbolTable> & symbol_table, int indent) {
    print_indent(indent);
    printf("[dump] Package(%s):\n", symbol_table->id2name(name_id).c_str());
    auto iter = data.begin();
    for (; iter!=data.end(); iter++) {
        print_indent(indent+1);
        printf("%s:\n", symbol_table->id2name(iter->first).c_str());
        iter->second->dump(indent+2);
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

