#include "vm.h"
// #include "package.h"

using namespace tora;

void Package::dump(SharedPtr<SymbolTable> & symbol_table, int indent) {
    print_indent(indent);
    printf("[dump] Package(%s):\n", symbol_table->id2name(name_id).c_str());
    auto iter = data.begin();
    for (; iter!=data.end(); iter++) {
        print_indent(indent+1);
        printf("%s:\n", symbol_table->id2name(iter->first).c_str());
        iter->second->dump(indent+2);
    }
}
