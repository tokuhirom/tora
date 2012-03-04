#include <stdio.h>
#include "util.h"

void tora::print_indent(int indent) {
    for (int i=0; i<indent; i++) {
        printf("    ");
    }
}
