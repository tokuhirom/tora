#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <errno.h>
#include "util.h"

#ifdef _WIN32
#include <windows.h>
#endif

void tora::print_indent(int indent) {
  for (int i = 0; i < indent; i++) {
    printf("    ");
  }
}

bool tora::split_package_funname(const std::string &varname,
                                 std::string &pkgname, std::string &detail) {
  auto pos = varname.rfind("::");
  if (pos != std::string::npos) {
    // package variable Foo::Bar.
    pkgname = varname.substr(0, pos);
    detail = varname.substr(pos + 2);
    return true;
  } else {
    return false;
  }
}

bool tora::split_package_varname(const std::string &varname,
                                 std::string &pkgname, std::string &detail) {
  auto pos = varname.rfind("::");
  if (pos != std::string::npos) {
    // package variable $Foo::Bar.
    pkgname = varname.substr(1, pos - 1);
    detail = varname.substr(pos + 2);
    return true;
  } else {
    return false;
  }
}

int tora::hexchar2int(unsigned char c) {
  if ('0' <= c && c <= '9') {
    return c - '0';
  } else if ('a' <= c && c <= 'f') {
    return c - 'a' + 10;
  } else if ('A' <= c && c <= 'F') {
    return c - 'A' + 10;
  } else {
    printf("invalid %c\n", c);
    abort();
  }
}

int tora::octchar2int(unsigned char c) { return c - '0'; }

void tora::set_errno(int err) {
#ifdef _WIN32
  SetLastError(err);
#else
  errno = err;
#endif
}

int tora::get_errno() {
#ifdef _WIN32
  return GetLastError();
#else
  return errno;
#endif
}

std::string tora::get_strerror(int err) {
#ifdef _WIN32
  static char buf[256];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf,
                sizeof buf, NULL);
  return std::string(buf);
#else
  return std::string(strerror(err));
#endif
}
