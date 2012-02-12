#include "hash.h"

using namespace tora;

void HashValue::dump() {
    printf("[dump] hash(%zd)\n", this->data.size());
    for (auto iter = this->data.begin(); iter!=this->data.end(); iter++) {
        printf("%s: ", iter->first.c_str());
        iter->second->dump();
    }
}
