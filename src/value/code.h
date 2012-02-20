#ifndef TORA_CODE_H_
#define TORA_CODE_H_

#include <cstdio>
#include <vector>
#include "../value.h"
#include "../op.h"
#include "../shared_ptr.h"

namespace tora {

class CodeValue: public Value {
public:
    ID code_id;
    std::string code_name;
    std::vector<std::string*> *code_params;
    std::vector<SharedPtr<OP>> *code_opcodes;

    CodeValue(): Value() {
        this->value_type = VALUE_TYPE_CODE;
    }
    ~CodeValue() {
        delete code_params;
        delete code_opcodes;
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

