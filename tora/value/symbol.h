#ifndef TORA_VALUE_SYMBOL_H_
#define TORA_VALUE_SYMBOL_H_

#include "../value.h"
#include "../tora.h"

namespace tora {

  class VM;

  class MortalSymbolValue : public MortalValue {
    static Value* new_value(ID id) {
      Value* v = new Value(VALUE_TYPE_SYMBOL);
      v->id_value_ = id;
      return v;
    }
  public:
    MortalSymbolValue(ID id)
      : MortalValue(new_value(id)) { }
  };

  ID symbol_id(const Value* self);
  std::string symbol_name(VM* vm, const Value* self);

}

#endif  // TORA_VALUE_SYMBOL_H_
