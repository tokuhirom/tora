#ifndef TORA_UTIL_H_
#define TORA_UTIL_H_

#include <string>

namespace tora {

void print_indent(int indent);

/**
 * Split "$Foo::Bar" to "$Foo" and "Bar"
 * split package to package name and moniker.
 */
bool split_package_varname(const std::string & src, std::string & pkgname, std::string &detail);

/**
 * split "Foo::Bar::baz" to "Foo::Bar" and "baz"
 */
bool split_package_funname(const std::string & src, std::string & pkgname, std::string &detail);

};

#endif
