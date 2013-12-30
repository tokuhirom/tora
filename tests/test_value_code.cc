#include "nanotap.h"
#include "../tora/value/code.h"
#include "../tora/symbol_table.h"
#include <stdarg.h>
#include <vector>
#include <string>

using namespace tora;

int main() {
    {
        SharedPtr<SymbolTable> symbol_table = new SymbolTable();
        std::string filename_("<eval>");
        int lineno = 3;
        std::shared_ptr<std::vector<std::string>> params = std::make_shared<std::vector<std::string>>();

        MortalCodeValue code(
            symbol_table->get_id("HOGE"), // package id
            symbol_table->get_id("FUGA"),        // func name id
            filename_,
            lineno,
            params
        );
        is(code_package_id(code.get()), symbol_table->get_id("HOGE"));
    }

    done_testing();
}
