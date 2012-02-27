#ifndef TORA_SYMBOL_TABLE_H_
#define TORA_SYMBOL_TABLE_H_

#include <map>
#include "prim.h"
#include "tora.h"

namespace tora {

class SymbolTable : public Prim {
    int counter;
    std::map<std::string, int> tbl;
    std::map<int, std::string> rev_tbl; // reverse table
public:
    SymbolTable() : counter(0) { }
    
    /**
     * Convert :symbol to ID.
     */
    ID get_id(const char *str) {
        return this->get_id(std::string(str));
    }
    ID get_id(const std::string &str);
    /**
     * Convert ID to :symbol.
     */
    std::string &id2name(int id) {
        return rev_tbl[id];
    }
};

};

#endif // TORA_SYMBOL_TABLE_H_
