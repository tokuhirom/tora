#include "nanotap.h"
#include "../tora/compiler.h"
#include "../tora/symbol_table.h"
#include "../tora/value.h"
#include "../tora/node.h"
#include <stdarg.h>

using namespace tora;

int main() {
    {
        std::shared_ptr<SymbolTable> st = std::make_shared<SymbolTable>();
        Compiler compiler(st, "<eval>");
        SharedPtr<Node> n = new FuncdefNode(new StrNode(NODE_IDENTIFIER, "hoge"), new Node(), new Node(NODE_VOID));
        compiler.compile(n);
        ok(1);
    }

    done_testing();
}

