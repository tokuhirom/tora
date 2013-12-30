#include "code.h"
#include "../vm.h"
#include "../callback.h"

using namespace tora;

static CodeValue* get_code(const Value* v)
{
  assert(type(v) == VALUE_TYPE_CODE);
  return static_cast<CodeValue*>(const_cast<Value*>(v));
// return static_cast<CodeValue*>(get_ptr_value(v));
}

std::string tora::code_filename(const Value *v)
{
  return get_code(v)->filename;
}

int tora::code_lineno(const Value *v)
{
  return get_code(v)->lineno;
}

ID tora::code_func_name_id(const Value* v)
{
  return get_code(v)->func_name_id;
}

CodeValue::~CodeValue() {
  /*
  if (code_params_) {
      auto iter = code_params_->begin();
      for (; iter!=code_params_->end(); iter++) {
          delete *iter;
      }
      delete code_params_;
      code_params_ = NULL;
  }
  */

  if (is_native_ && callback_) {
    delete callback_;
    callback_ = NULL;
  }

  /*
  delete closure_var_names_;
  closure_var_names_ = NULL;

  if (closure_vars_) {
      delete closure_vars_;
      closure_vars_ = NULL;
  }
  */
}

/*
void CodeValue::dump(SharedPtr<SymbolTable> & symbol_table, int indent) {
    print_indent(indent);
    printf("[dump] code: name: %s::%s\n",
symbol_table->id2name(this->package_id).c_str(),
symbol_table->id2name(this->func_name_id).c_str());
    for (size_t i=0; i<this->code_opcodes->size(); i++) {
        print_indent(indent+1);
        printf("[%ld] %s\n", (long int) i,
opcode2name[this->code_opcodes->at(i)->op_type]);
    }
    printf("----------------\n");
}
*/
