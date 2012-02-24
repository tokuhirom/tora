#ifndef TORA_CODE_H_
#define TORA_CODE_H_

#include <cstdio>
#include <vector>
#include <string>
#include "../value.h"
#include "../op.h"
#include "../shared_ptr.h"
#include "../op_array.h"

namespace tora {

class CodeValue: public Value {
public:
    ID package_id;
    ID func_name_id;
    ID code_id;
    std::string code_name;
    std::vector<std::string*> *code_params;
    std::vector<std::string> *closure_var_names;
    std::vector<SharedPtr<Value>> *closure_vars;
    SharedPtr<OPArray> code_opcodes;

    CodeValue(): Value(VALUE_TYPE_CODE) {
        this->closure_vars = new std::vector<SharedPtr<Value>>();
    }
    ~CodeValue() {
        auto iter = code_params->begin();
        for (; iter!=code_params->end(); iter++) {
            delete *iter;
        }

        delete closure_var_names;
        delete closure_vars;

        delete code_params;
    }
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] code: name: %s\n", this->code_name.c_str());
        for (size_t i=0; i<this->code_opcodes->size(); i++) {
            print_indent(indent+1);
            printf("[%zd] %s\n", i, opcode2name[this->code_opcodes->at(i)->op_type]);
        }
        printf("----------------\n");
    }
    const char *type_str() { return "code"; }
    SharedPtr<StrValue> to_s();
};

};

#endif // TORA_CODE_H_

