#include "peek.h"
#include "value.h"
#include "value/symbol.h"
#include "value/code.h"
#include "value/class.h"
#include "vm.h"

using namespace tora;

void tora::peek(VM *vm, const SharedPtr<Value>& v) {
    peek(vm, v.get());
}

// vm is nullable.
void tora::peek(VM *vm, const Value * v) {
    if (v) {
        printf("Value: %s(%p)\n", v->type_str(), v);
        /*
        printf("  CODE SIZE: %ld\n", (long int) sizeof(CodeValue));
        printf("  STR SIZE: %ld\n", (long int) sizeof(StrValue));
        printf("  VALUE SIZE: %ld\n", (long int) sizeof(Value));
        printf("  INT SIZE: %ld\n", (long int) sizeof(IntValue));
        */
        printf("  REFCNT: %d\n", v->refcnt);
        switch (v->value_type) {
        case VALUE_TYPE_SYMBOL: {
            ID id = static_cast<const SymbolValue*>(v)->id();
            printf("  ID: %d(%s)\n", id, vm ? vm->symbol_table->id2name(id).c_str() : "NO Symbol table given");
            break;
        }
        case VALUE_TYPE_INT: {
            printf("  INT: %d\n", static_cast<const IntValue*>(v)->int_value());
            break;
        }
        case VALUE_TYPE_CODE: {
            if (vm) {
                printf("  PACKAGE: %s\n", vm->symbol_table->id2name(static_cast<const CodeValue*>(v)->package_id()).c_str());
                printf("  FUNC_NAME: %s\n", vm->symbol_table->id2name(static_cast<const CodeValue*>(v)->func_name_id()).c_str());
            } else {
                printf("  PACKAGE_ID: %d\n", static_cast<const CodeValue*>(v)->package_id());
                printf("  FUNC_NAME_ID: %d\n", static_cast<const CodeValue*>(v)->func_name_id());
            }
            break;
        }
        case VALUE_TYPE_ARRAY: {
            const ArrayValue* av = static_cast<const ArrayValue*>(v);
            printf("  SIZE: %ld\n", (long int) av->size());
            break;
        }
        case VALUE_TYPE_CLASS: {
            const ClassValue*cv = static_cast<const ClassValue*>(v);
            printf("  NAME: %s\n", cv->name().c_str());
            break;
        }
        case VALUE_TYPE_UNDEF:
        case VALUE_TYPE_BOOL:
        case VALUE_TYPE_STR:
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
        case VALUE_TYPE_REFERENCE:
        case VALUE_TYPE_FILE_PACKAGE:
        case VALUE_TYPE_BYTES:
            // nop.
            break;
        }
    } else {
        printf("(null)\n");
    }
}
