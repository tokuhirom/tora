#include "code.h"
#include "../vm.h"
#include "../callback.h"
#include "../pad_list.h"
#include "../op_array.h"
#include "../shared_ptr.h"
#include "../op.h"

using namespace tora;

struct CodeImpl {
  std::shared_ptr<CallbackFunction> callback;
  bool is_native;
  ID package_id;
  ID func_name_id;
  // ID code_id_;
  // std::string code_name_;
  std::shared_ptr<std::vector<std::string>> code_params;
  std::shared_ptr<std::vector<int>> code_defaults;
  std::shared_ptr<std::vector<std::string>> closure_var_names;
  std::vector<SharedValue> closure_vars;
  std::shared_ptr<PadList> pad_list;
  std::shared_ptr<OPArray> code_opcodes;
  std::string filename;

  // line number.
  // It's -1 if it's native function
  int lineno;
};

static CodeImpl* get_code(const Value* v)
{
  assert(type(v) == VALUE_TYPE_CODE);
  return static_cast<CodeImpl*>(get_ptr_value(v));
}

void tora::code_free(Value* self)
{
  delete get_code(self);
}

void tora::code_defaults(Value* self, const std::shared_ptr<std::vector<int>> &c)
{
  get_code(self)->code_defaults = c;
}

std::shared_ptr<std::vector<int>> tora::code_defaults(Value* self)
{
  return get_code(self)->code_defaults;
}

CallbackFunction *tora::code_callback(Value* self)
{
  return get_code(self)->callback.get();
}

void tora::code_params(Value* self, const std::shared_ptr<std::vector<std::string>> &v)
{
  get_code(self)->code_params = v;
}

std::shared_ptr<std::vector<std::string>> tora::code_params(Value* self)
{
  return get_code(self)->code_params;
}

void tora::code_opcodes(Value* self, const std::shared_ptr<OPArray> &v)
{
  get_code(self)->code_opcodes = v;
}

const std::shared_ptr<OPArray> &tora::code_opcodes(Value* self)
{
  return get_code(self)->code_opcodes;
}

void tora::code_closure_var_names(Value* self, const std::shared_ptr<std::vector<std::string>> &closure_var_names__)
{
  get_code(self)->closure_var_names = closure_var_names__;
}

std::shared_ptr<std::vector<std::string>> tora::code_closure_var_names(Value* self)
{
  return get_code(self)->closure_var_names;
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

const std::shared_ptr<PadList> &tora::code_pad_list(Value* v)
{
  return get_code(v)->pad_list;
}

void tora::code_pad_list(Value* v, const std::shared_ptr<PadList> &p)
{
  get_code(v)->pad_list = p;
}

void tora::code_package_id(Value* self, ID id)
{
  get_code(self)->package_id = id;
}

ID tora::code_package_id(Value* self)
{
  return get_code(self)->package_id;
}

bool tora::code_is_native(Value* self)
{
  return get_code(self)->is_native;
}

static Value* new_tora_code(
    ID _package_id, ID _func_name_id, const std::string &_filename, int _lineno,
    const std::shared_ptr<std::vector<std::string>> &_code_params)
{
  CodeImpl* p = new CodeImpl;
  p->callback = NULL;
  p->is_native = false;
  p->package_id = _package_id;
  p->func_name_id = _func_name_id;
  p->code_params = _code_params;
  p->filename = _filename;
  p->lineno = _lineno;
  return new Value(VALUE_TYPE_CODE, static_cast<void*>(p));
}

static Value* new_cpp_code(
  ID _package_id, ID _func_name_id, const std::shared_ptr<CallbackFunction>& _cb
) {
  CodeImpl* p = new CodeImpl;
  p->callback = _cb;
  p->is_native = true;
  p->package_id = _package_id;
  p->func_name_id = _func_name_id;
  p->lineno = -1;
  return new Value(VALUE_TYPE_CODE, static_cast<void*>(p));
}

// for tora functions
MortalCodeValue::MortalCodeValue(
      ID _package_id, ID _func_name_id, const std::string &_filename, int _lineno,
      const std::shared_ptr<std::vector<std::string>> &_code_params)
  : MortalValue(new_tora_code(_package_id, _func_name_id, _filename, _lineno, _code_params)) { }

// for C++ functions
MortalCodeValue::MortalCodeValue(
  ID _package_id, ID _func_name_id, const std::shared_ptr<CallbackFunction>& _cb
) : MortalValue(new_cpp_code(_package_id, _func_name_id, _cb)) { }

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
