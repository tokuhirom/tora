#include "peek.h"
#include "value.h"
#include "value/symbol.h"
#include "vm.h"

using namespace tora;

void tora::peek(VM *vm, const Value * v) {
    if (v) {
        printf("Value: %s\n", v->type_str());
        printf("  REFCNT: %d\n", v->refcnt);
        switch (v->value_type) {
        case VALUE_TYPE_SYMBOL: {
            ID id = static_cast<const SymbolValue*>(v)->id;
            printf("  ID: %d(%s)\n", id, vm ? vm->symbol_table->id2name(id).c_str() : "NO Symbol table given");
            break;
        }
        case VALUE_TYPE_INT: {
            printf("  INT: %d\n", static_cast<const IntValue*>(v)->int_value);
            break;
        }
        case VALUE_TYPE_UNDEF:
        case VALUE_TYPE_BOOL:
        case VALUE_TYPE_STR:
        case VALUE_TYPE_CODE:
        case VALUE_TYPE_ARRAY:
        case VALUE_TYPE_DOUBLE:
        case VALUE_TYPE_REGEXP:
        case VALUE_TYPE_TUPLE:
        case VALUE_TYPE_FILE:
        case VALUE_TYPE_RANGE:
        case VALUE_TYPE_ARRAY_ITERATOR:
        case VALUE_TYPE_RANGE_ITERATOR:
        case VALUE_TYPE_EXCEPTION:
        case VALUE_TYPE_HASH:
        case VALUE_TYPE_HASH_ITERATOR:
        case VALUE_TYPE_OBJECT:
        case VALUE_TYPE_POINTER:
            // nop.
            break;
        }
    } else {
        printf("(null)\n");
    }
}
