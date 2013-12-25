#ifndef TORA_VALUE_SYMBOL_H_
#define TORA_VALUE_SYMBOL_H_

#include "../value.h"
#include "../tora.h"

namespace tora {

class SymbolValue : public Value {
 public:
  SymbolValue(ID id) : Value(VALUE_TYPE_SYMBOL) { id_value_ = id; }
  ~SymbolValue() {}
  ID id() const { return id_value_; }
};
}

#endif  // TORA_VALUE_SYMBOL_H_
