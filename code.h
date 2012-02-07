#ifndef TORA_CODE_H_
#define TORA_CODE_H_

#include <cstdio>
#include <vector>
#include "value.h"
#include "op.h"
#include "shared_ptr.h"

namespace tora {

class CodeValue: public Value {
public:
    const char *code_name;
    std::vector<std::string*> *code_params;
    std::vector<SharedPtr<OP>> *code_opcodes;

    CodeValue(): Value() {
        this->value_type = VALUE_TYPE_CODE;
    }
    ~CodeValue() {
        free((void*)code_name);
        delete code_params;
        delete code_opcodes;
    }
    void dump() {
        printf("[dump] code: name: %s\n", this->code_name);
        for (size_t i=0; i<this->code_opcodes->size(); i++) {
            printf("    [%d] %s\n", i, opcode2name[this->code_opcodes->at(i)->op_type]);
        }
        printf("----------------\n");
    }
    const char *type_str() { return "code"; }
    SharedPtr<StrValue> to_s();
};

};

#endif // TORA_CODE_H_

