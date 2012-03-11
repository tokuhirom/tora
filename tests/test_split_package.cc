#include "../src/util.h"
#include "nanotap.h"

using namespace tora;

int main() {
    std::string package;
    std::string moniker;
    is(split_package(std::string("$Foo::Bar"), package, moniker), true);
    is(package, "Foo");
    is(moniker, "Bar");
    done_testing();
}

