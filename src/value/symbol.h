#ifndef TORA_VALUE_SYMBOL_H_
#define TORA_VALUE_SYMBOL_H_

#include "../value.h"
#include "../tora.h"

namespace tora {

class SymbolValue: public Value {
public:
    ID id;
    SymbolValue(ID i): Value(VALUE_TYPE_SYMBOL), id(i) {
    }
    ~SymbolValue() { }
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] Symbol: %d\n", id);
    }
    const char *type_str() { return "symbol"; }
};

}

#endif // TORA_VALUE_SYMBOL_H_
