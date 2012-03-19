#include <stdio.h>
#include <string>
#include "util.h"

void tora::print_indent(int indent) {
    for (int i=0; i<indent; i++) {
        printf("    ");
    }
}

bool tora::split_package_funname(const std::string & varname, std::string & pkgname, std::string &detail) {
    auto pos = varname.rfind("::");
    if (pos != std::string::npos) {
        // package variable Foo::Bar.
        pkgname = varname.substr(0, pos);
        detail = varname.substr(pos+2);
        return true;
    } else {
        return false;
    }
}

bool tora::split_package_varname(const std::string & varname, std::string & pkgname, std::string &detail) {
    auto pos = varname.rfind("::");
    if (pos != std::string::npos) {
        // package variable $Foo::Bar.
        pkgname = varname.substr(1, pos-1);
        detail = varname.substr(pos+2);
        return true;
    } else {
        return false;
    }
}

