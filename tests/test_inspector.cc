#include "nanotap.h"
#include "../tora/value.h"
#include "../tora/value/array.h"
#include "../tora/value/hash.h"
#include "../tora/value/int.h"
#include "../tora/value/range.h"
#include "../tora/vm.h"
#include "../tora/inspector.h"

using namespace tora;

int main() {
    SharedPtr<OPArray> oparray = new OPArray();
    SharedPtr<SymbolTable> symbol_table = new SymbolTable();
    VM vm(oparray, symbol_table, false);
    Inspector ins(&vm);

    {
        SharedPtr<Value> v = new IntValue(4649);
        is(ins.inspect(v), std::string("4649"));
    }

    {
      MortalArrayValue av;
      MortalIntValue i5963(5963);
      array_push_back(av.get(), i5963.get());
      array_push_back(av.get(), vm.to_bool(true));
      is(ins.inspect(av.get()), std::string("[5963,true]"));
    }

    {
      MortalHashValue hv;
      MortalIntValue i5963(5963);
      MortalIntValue i4649(4649);
      hash_set_item(hv.get(), "HOGE", i5963.get());
      hash_set_item(hv.get(), "IYAN", i4649.get());
      is(ins.inspect(hv.get()), std::string("{\"HOGE\" => 5963,\"IYAN\" => 4649}"));
    }

    {
      MortalRangeValue rv(3, 9);
      is(ins.inspect(rv.get()), std::string("3..9"));
  }

    done_testing();
}
