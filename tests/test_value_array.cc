#include "nanotap.h"
#include "../tora/compiler.h"
#include "../tora/symbol_table.h"
#include "../tora/value.h"
#include "../tora/value/array.h"
#include "../tora/value/int.h"
#include "../tora/node.h"
#include <stdarg.h>

using namespace tora;

int main() {
  MortalArrayValue av;
  is(array_size(av.get()), (tra_int)0);
  {
    MortalIntValue i1(49);
    array_push_back(av.get(), i1.get());
    is(array_size(av.get()), (tra_int) 1UL);
  }
  {
    MortalIntValue v(55);
    array_set_item(av.get(), 1, v.get());
    is(get_int_value(array_get_item(av.get(), 1)), 55);
  }
  {
    MortalIntValue v(88);
    array_set_item(av.get(), 1, v.get());
  }
  is(get_int_value(array_get_item(av.get(), 1)), 88);
  // is((void*)v.get(), (void*)av->at(1).get());
  is(array_size(av.get()), (tra_int) 2UL);

  done_testing();
}

