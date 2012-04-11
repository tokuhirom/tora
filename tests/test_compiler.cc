#include "nanotap.h"
#include "../tora/compiler.h"
#include "../tora/symbol_table.h"
#include "../tora/value.h"
#include "../tora/node.h"
#include <stdarg.h>

using namespace tora;

int main() {
    {
        SharedPtr<SymbolTable> st = new SymbolTable();
        Compiler compiler(st, "<eval>");
        SharedPtr<Node> n = new FuncdefNode(new StrNode(NODE_IDENTIFIER, "hoge"), new Node(), new Node(NODE_VOID));
        compiler.compile(n);
    }

    done_testing();
}

