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

struct CallbackFunction;
class SymbolTable;

class CodeValue: public Value {
    const CallbackFunction * callback_;
    bool is_native_;
    ID package_id;
    ID func_name_id;
    ID code_id;
    std::string code_name;
    std::vector<std::string*> *code_params;
    std::vector<int> *code_defaults;
    std::vector<std::string> *closure_var_names;
    std::vector<SharedPtr<Value>> *closure_vars;
    SharedPtr<OPArray> code_opcodes;
    std::string filename;
public:

    // line number.
    // It's -1 if it's native function
    int lineno;

    bool is_native() const { return is_native_; }
    const CallbackFunction* callback() const { return callback_; }

    // for tora functions
    CodeValue(ID package_id_, ID func_name_id_, const std::string &filename_, int lineno_): Value(VALUE_TYPE_CODE), callback_(NULL), is_native_(false), package_id(package_id_), func_name_id(func_name_id_), filename(filename_), lineno(lineno_) {
        this->closure_vars = new std::vector<SharedPtr<Value>>();
    }
    // for C++ functions
    CodeValue(ID package_id_, ID func_name_id_, const CallbackFunction * cb): Value(VALUE_TYPE_CODE), callback_(cb), is_native_(true), package_id(package_id_), func_name_id(func_name_id_), code_params(NULL), closure_var_names(NULL), lineno(-1) {
        this->closure_vars = new std::vector<SharedPtr<Value>>();
    }
    ~CodeValue();

    void dump(SharedPtr<SymbolTable> & symbol_table, int indent);
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] code: name: %s\n", this->code_name.c_str());
        for (size_t i=0; i<this->code_opcodes->size(); i++) {
            print_indent(indent+1);
            printf("[%zd] %s\n", i, opcode2name[this->code_opcodes->at(i)->op_type]);
        }
        printf("----------------\n");
    }
};

};

#endif // TORA_CODE_H_

