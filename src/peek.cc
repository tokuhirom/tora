#include "peek.h"
#include "value.h"

using namespace tora;

void tora::peek(const Value * v) {
    if (v) {
        printf("Value: %s\n", v->type_str());
        printf("  REFCNT: %d\n", v->refcnt);
    } else {
        printf("(null)\n");
    }
}
