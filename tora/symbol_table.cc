#include "symbol_table.h"
#ifndef _WIN32
#include <execinfo.h>
#endif
#include <iostream>

using namespace tora;

ID SymbolTable::get_id(const std::string &str) {
  // find symbol from table
  auto i = this->tbl.find(str);
  if (i != this->tbl.end()) {
    return i->second;
  }

  // create new symbol
  int ret = counter++;
  tbl[str] = ret;
  rev_tbl[ret] = str;
  return ret;
}

void SymbolTable::dump() const {
  for (auto iter = tbl.begin(); iter != tbl.end(); iter++) {
    std::cout << iter->first << " " << iter->second << std::endl;
  }
}
