#include "nanotap.h"
#include "../tora/value.h"
#include "../tora/value/array.h"
#include "../tora/value/hash.h"
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
        SharedPtr<ArrayValue> av = new ArrayValue();
        av->push_back(new IntValue(5963));
        av->push_back(vm.to_bool(true));
        is(ins.inspect(av), std::string("[5963,true]"));
    }

    {
      MortalHashValue hv;
      hash_set_item(hv.get(), "HOGE", new IntValue(5963));
      hash_set_item(hv.get(), "IYAN", new IntValue(4649));
      is(ins.inspect(hv.get()), std::string("{\"HOGE\" => 5963,\"IYAN\" => 4649}"));
    }

    {
      MortalRangeValue rv(3, 9);
      is(ins.inspect(rv.get()), std::string("3..9"));
  }

    done_testing();
}
