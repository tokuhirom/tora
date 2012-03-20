#include "../tora/util.h"
#include "nanotap.h"

using namespace tora;

int main() {
    {
        std::string package;
        std::string moniker;
        is(split_package_varname(std::string("$Foo::Bar"), package, moniker), true);
        is(package, "Foo");
        is(moniker, "Bar");
    }
    {
        std::string package;
        std::string moniker;
        is(split_package_funname(std::string("Foo::Bar"), package, moniker), true);
        is(package, "Foo");
        is(moniker, "Bar");
    }
    done_testing();
}

