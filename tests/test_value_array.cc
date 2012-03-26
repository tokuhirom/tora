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
    is(av->size(), 0UL);
    av->push_back(new IntValue(49));
    is(av->size(), 1UL);
    SharedPtr<Value> v = new IntValue(55);
    av->set_item(1, v);
    is(av->at(1)->upcast<IntValue>()->int_value(), 55);
    av->set_item(1, new IntValue(88));
    is(av->at(1)->upcast<IntValue>()->int_value(), 88);
    is((void*)v.get(), (void*)av->at(1).get());
    is(av->size(), 2UL);

    done_testing();
}

