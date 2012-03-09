#include "nanotap.h"
#include "../src/value.h"
#include "../src/value/array.h"
#include "../src/value/hash.h"
#include "../src/value/range.h"
#include "../src/vm.h"
#include "../src/inspector.h"

using namespace tora;

int main() {
    SharedPtr<OPArray> oparray = new OPArray();
    SharedPtr<SymbolTable> symbol_table = new SymbolTable();
    VM vm(oparray, symbol_table);
    Inspector ins(&vm);

    {
        SharedPtr<Value> v = new IntValue(4649);
        is(ins.inspect(v), std::string("4649"));
    }

    {
        SharedPtr<ArrayValue> av = new ArrayValue();
        av->push(new IntValue(5963));
        av->push(new BoolValue(true));
        is(ins.inspect(av), std::string("[5963,true]"));
    }

    {
        SharedPtr<HashValue> hv = new HashValue();
        hv->set("HOGE", new IntValue(5963));
        hv->set("IYAN", new IntValue(4649));
        is(ins.inspect(hv), std::string("{\"HOGE\" => 5963,\"IYAN\" => 4649}"));
    }

    {
        SharedPtr<RangeValue> rv = new RangeValue(new IntValue(3), new IntValue(9));
        is(ins.inspect(rv), std::string("3..9"));
    }

    done_testing();
}
