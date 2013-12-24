#include "nanotap.h"
#include "../tora/compiler.h"
#include "../tora/symbol_table.h"
#include "../tora/value.h"
#include "../tora/value/array.h"
#include "../tora/node.h"
#include <stdarg.h>

using namespace tora;

int main() {
    SharedPtr<ArrayValue> av = new ArrayValue();
    is(av->size(), (size_t) 0UL);
    av->push_back(new IntValue(49));
    is(av->size(), (size_t) 1UL);
    SharedPtr<Value> v = new IntValue(55);
    av->set_item(1, v);
    is(get_int_value(av->at(1)), 55);
    av->set_item(1, new IntValue(88));
    is(get_int_value(av->at(1)), 88);
    // is((void*)v.get(), (void*)av->at(1).get());
    is(av->size(), (size_t) 2UL);

    done_testing();
}

