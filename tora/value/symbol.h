#ifndef TORA_VALUE_SYMBOL_H_
#define TORA_VALUE_SYMBOL_H_

#include "../value.h"
#include "../tora.h"

namespace tora {

class SymbolValue: public Value {
public:
    SymbolValue(ID i): Value(VALUE_TYPE_SYMBOL, i) {
    }
    ~SymbolValue() { }
    ID id() const {
        return boost::get<ID>(value_);
    }
};

}

#endif // TORA_VALUE_SYMBOL_H_
