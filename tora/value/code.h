#ifndef TORA_CODE_H_
#define TORA_CODE_H_

#include <vector>
#include <string>
#include <memory>
#include "../value.h"

namespace tora {

class CallbackFunction;
class SymbolTable;
class PadList;
class OPArray;

class MortalCodeValue : public MortalValue {
public:
  // for tora functions
  explicit MortalCodeValue(
      ID package_id, ID _func_name_id, const std::string &_filename, int _lineno,
      const std::shared_ptr<std::vector<std::string>> &code_params);

  // for C++ functions
  explicit MortalCodeValue(ID package_id, ID _func_name_id, const std::shared_ptr<CallbackFunction>& _cb);
};

std::string code_filename(const Value *v);
int code_lineno(const Value *v);
ID code_func_name_id(const Value* v);
const std::shared_ptr<PadList> &code_pad_list(Value* v);
void code_pad_list(Value* self, const std::shared_ptr<PadList> &p);
std::vector<SharedValue> code_closure_vars(Value* self);
ID code_package_id(Value* self);
void code_package_id(Value* self, ID id);
void code_opcodes(Value* self, const std::shared_ptr<OPArray> &v);
const std::shared_ptr<OPArray> &code_opcodes(Value* self);
void code_params(Value* self, const std::shared_ptr<std::vector<std::string>> &v);
std::shared_ptr<std::vector<std::string>> code_params(Value* self);
bool code_is_native(Value* self);
CallbackFunction *code_callback(Value* self);
void code_defaults(Value* self, const std::shared_ptr<std::vector<int>> &c);
std::shared_ptr<std::vector<int>> code_defaults(Value* self);
void code_closure_var_names(Value* self, const std::shared_ptr<std::vector<std::string>> &closure_var_names__);
std::shared_ptr<std::vector<std::string>> code_closure_var_names(Value* self);
void code_free(Value* self);

};

#endif  // TORA_CODE_H_
