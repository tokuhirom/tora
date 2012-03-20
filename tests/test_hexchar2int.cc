#include "nanotap.h"
#include "../tora/util.h"
#include <stdarg.h>

using namespace tora;

int main() {
    is(hexchar2int('0'), 0);
    is('1' - '0', 1);
    is(hexchar2int('1'), 1);
    is(hexchar2int('9'), 9);
    is(hexchar2int('a'), 10);
    is(hexchar2int('f'), 15);
    is(hexchar2int('A'), 10);
    is(hexchar2int('F'), 15);
    done_testing;
}

