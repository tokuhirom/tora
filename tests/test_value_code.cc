#include "nanotap.h"
#include "../tora/value/code.h"
#include "../tora/symbol_table.h"
#include <stdarg.h>

using namespace tora;

int main() {
    SymbolTable symbol_table;
    std::string filename_("foo.tra");
    int lineno = 3;

    SharedPtr<CodeValue> code = new CodeValue(
        symbol_table.get_id("HOGE"), // package id
        symbol_table.get_id("FUGA"),        // func name id
        filename_,
        lineno
    );
    is(code->package_id(), symbol_table.get_id("HOGE"));
    done_testing();
}
