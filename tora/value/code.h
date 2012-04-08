#ifndef TORA_CODE_H_
#define TORA_CODE_H_

#include <cstdio>
#include <vector>
#include <string>
#include "../value.h"
#include "../op.h"
#include "../shared_ptr.h"
#include "../op_array.h"
#include "../pad_list.h"

#include <boost/shared_ptr.hpp>

namespace tora {

class CallbackFunction;
class SymbolTable;

class CodeValue: public Value {
private:
    // line number.
    // It's -1 if it's native function
    const CallbackFunction * callback_;
    bool is_native_;
    ID package_id_;
    ID func_name_id_;
    // ID code_id_;
    // std::string code_name_;
    boost::shared_ptr<std::vector<std::string>> code_params_;
    boost::shared_ptr<std::vector<int>> code_defaults_;
    boost::shared_ptr<std::vector<std::string>> closure_var_names_;
    std::vector<SharedPtr<Value>> closure_vars_;
    SharedPtr<PadList> pad_list_;
    SharedPtr<OPArray> code_opcodes_;
    std::string filename_;
    int lineno_;
public:

    bool is_native() const { return is_native_; }
    const CallbackFunction* callback() const { return callback_; }

    // for tora functions
    explicit CodeValue(ID package_id, ID func_name_id, const std::string &filename, int lineno, const boost::shared_ptr<std::vector<std::string>> & code_params)
        : Value(VALUE_TYPE_CODE)
        , callback_(NULL)
        , is_native_(false)
        , package_id_(package_id)
        , func_name_id_(func_name_id)
        , code_params_(code_params)
        // , closure_var_names_(NULL)
        , filename_(filename)
        , lineno_(lineno)
        {
    }

    // for C++ functions
    CodeValue(ID package_id, ID func_name_id, const CallbackFunction * cb)
        : Value(VALUE_TYPE_CODE)
        , callback_(cb)
        , is_native_(true)
        , package_id_(package_id)
        , func_name_id_(func_name_id)
        // , code_params_(NULL)
        // , closure_var_names_(NULL)
        , lineno_(-1) {
    }
    ~CodeValue();

    void code_params(const boost::shared_ptr<std::vector<std::string>> & v) {
        code_params_ = v;
    }
    boost::shared_ptr<std::vector<std::string>> code_params() const {
        return code_params_;
    }
    void code_opcodes(const SharedPtr<OPArray>&v) {
        code_opcodes_ = v;
    }
    const SharedPtr<OPArray>& code_opcodes() const {
        return code_opcodes_;
    }
    void closure_var_names(const boost::shared_ptr<std::vector<std::string>> &closure_var_names__) {
        closure_var_names_ = closure_var_names__;
    }
    boost::shared_ptr<std::vector<std::string>> closure_var_names() const {
        return closure_var_names_;
    }
    void code_defaults(const boost::shared_ptr<std::vector<int>> &c) {
        code_defaults_ = c;
    }
    boost::shared_ptr<std::vector<int>> code_defaults() const {
        return code_defaults_;
    }
    ID package_id() const { return package_id_; }
    void package_id(ID id) { package_id_ = id; }
    
    ID func_name_id() const { return func_name_id_; }
    void func_name_id(ID id) { func_name_id_ = id; }

    int lineno() const { return lineno_; }
    const std::string & filename() const { return filename_; }

    void pad_list(const SharedPtr<PadList> & p) {
        pad_list_.reset(p.get());
    }
    const SharedPtr<PadList>& pad_list() const {
        return pad_list_;
    }
    std::vector<SharedPtr<Value>> &closure_vars() {
        return closure_vars_;
    }

    /*
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
    */
};

};

#endif // TORA_CODE_H_

