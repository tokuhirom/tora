#include "code.h"
#include "../vm.h"

using namespace tora;

CodeValue::~CodeValue() {
    if (code_params) {
        auto iter = code_params->begin();
        for (; iter!=code_params->end(); iter++) {
            delete *iter;
        }
        delete code_params;
    }

    if (is_native_) {
        delete callback_;
    }

    delete closure_var_names;
    delete closure_vars;
}

void CodeValue::dump(SharedPtr<SymbolTable> & symbol_table, int indent) {
    print_indent(indent);
    printf("[dump] code: name: %s::%s\n", symbol_table->id2name(this->package_id).c_str(), symbol_table->id2name(this->func_name_id).c_str());
    for (size_t i=0; i<this->code_opcodes->size(); i++) {
        print_indent(indent+1);
        printf("[%zd] %s\n", i, opcode2name[this->code_opcodes->at(i)->op_type]);
    }
    printf("----------------\n");
}

SharedPtr<StrValue> CodeValue::to_s() {
    return new StrValue("<code>"); // TODO
}

