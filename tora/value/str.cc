#include "str.h"
#include "../value.h"

using namespace tora;

int StrValue::length() {
    int len = 0;
    const std::string &str = *str_value_;

    for (int i=0; i<str.size(); ++i) {
        if ((str.at(i) & 0x80) == 0) { // 1 byte
            // printf("1 byte: %d\n", i);
            len++;
        } else {
            // printf("mutli byte! %d\n", i);
            len++;
            ++i;
            for (;  i<str.size() && (((unsigned char)str.at(i)) &0x80) ==0x80 && (str.at(i)&0x40) == 0; ++i) {
                /*
                printf("mutli byte: %d %x%x\n", i,
                    (unsigned char)this->str_value.at(i)&0x80,
                    (unsigned char)this->str_value.at(i)&0x40
                );
                */
                // skip
            }
        }
    }
    // printf("LEN: %d\n", len);
    return len;
}

