#include "symbol_table.h"
#include <execinfo.h>
#include <iostream>


using namespace tora;

ID SymbolTable::get_id(const std::string &str) {
    // find symbol from table
    auto i = this->tbl.find(str);
    if (i!=this->tbl.end()) {
        return i->second;
    }

    // create new symbol
    int ret = counter++;
    tbl[str] = ret;
    rev_tbl[ret] = str;
    return ret;
}

void SymbolTable::dump() const {
    for (auto iter: tbl) {
        std::cout << iter.first << " " << iter.second << std::endl;
    }
}

