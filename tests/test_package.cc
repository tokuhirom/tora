#include "nanotap.h"
#include "../tora/package.h"
#include "../tora/vm.h"
#include "../tora/symbol_table.h"

using namespace tora;

static SharedPtr<Value> foo_hoge(VM *vm) {
    return UndefValue::instance();
}

int main() {
    SharedPtr<SymbolTable> st = new SymbolTable();
    SharedPtr<OPArray> op_array = new OPArray();
    VM vm(op_array, st, false);
    SharedPtr<Package> pkg = new Package(&vm, st->get_id("Foo"));

    pkg->add_method(st->get_id("hoge"), new CallbackFunction(foo_hoge));
    ok(pkg->has_method(st->get_id("hoge")));
    ok(!pkg->has_method(st->get_id("hige")));

    done_testing();
}