#ifndef TORA_UTIL_H_
#define TORA_UTIL_H_

#include <string>

namespace tora {

void print_indent(int indent);

/**
 * split package to package name and moniker.
 */
bool split_package(const std::string & src, std::string & pkgname, std::string &detail);

};

#endif
