#include "nanotap.h"
#include "../tora/value/code.h"
#include "../tora/symbol_table.h"
#include <stdarg.h>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

using namespace tora;

int main() {
    {
        SharedPtr<SymbolTable> symbol_table = new SymbolTable();
        std::string filename_("<eval>");
        int lineno = 3;
        boost::shared_ptr<std::vector<std::string>> params(new std::vector<std::string>());

        SharedPtr<CodeValue> code = new CodeValue(
            symbol_table->get_id("HOGE"), // package id
            symbol_table->get_id("FUGA"),        // func name id
            filename_,
            lineno,
            params
        );
        is(code->package_id(), symbol_table->get_id("HOGE"));
    }

    done_testing();
}
