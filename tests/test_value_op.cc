#include "nanotap.h"
#include "../tora/op.h"
#include "../tora/value.h"
#include "../tora/symbol_table.h"
#include "../tora/op_array.h"
#include "../tora/value/code.h"
#include <stdarg.h>

using namespace tora;

int main() {
    {
         std::shared_ptr<SymbolTable> symbol_table = std::make_shared<SymbolTable>();
        std::shared_ptr<OPArray> op_array = std::make_shared<OPArray>();
        std::string filename("HOGE");
        int lineno = 3;
        std::shared_ptr<std::vector<std::string>> params(new std::vector<std::string>());
        MortalCodeValue code(
            symbol_table->get_id("hgoe"), // package id
            symbol_table->get_id("fuga"),        // func name id
            filename,
            lineno,
            params
        );
        is(code->refcnt(), 1);
        SharedPtr<ValueOP> op(new ValueOP(OP_PUSH_VALUE, code.get()));
        is(op->refcnt, 1);
    }
    ok(1);

    done_testing();
}

