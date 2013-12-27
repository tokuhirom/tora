#include "symbol.h"
#include "../vm.h"

using namespace tora;

ID tora::symbol_id(const Value* self) {
  assert(type(self) == VALUE_TYPE_SYMBOL);
  return self->id_value_;
}

std::string tora::symbol_name(VM* vm, const Value* self)
{
  return vm->symbol_table->id2name(symbol_id(self));
}
