#include "printf.h"
#include "value.h"
#include "value/exception.h"

#include <iostream>

using namespace tora;

std::string tora::tora_sprintf(const std::vector<SharedPtr<Value>> &args) {
  if (args.size() < 1) {
    throw new ArgumentExceptionValue("printf requires 1 or more args.");
  }

  int param_i = 1;
  const std::string pattern = args[0]->to_s();
  std::string ret;
  for (int i = 0; i < pattern.size(); i++) {
    if (pattern[i] == '%') {
      ++i;
      if (i >= pattern.size()) {
        throw new ExceptionValue("Bad printf pattern: '%s'", pattern.c_str());
      }
      switch (pattern[i]) {
        case 's':
          if (param_i >= args.size()) {
            throw new ExceptionValue("Bad printf pattern: '%s'",
                                     pattern.c_str());
          }
          // if (param_i>=
          ret += args[param_i++]->to_s();
          break;
        case 'd': {
          if (param_i >= args.size()) {
            throw new ExceptionValue("Bad printf pattern: '%s'",
                                     pattern.c_str());
          }
          // if (param_i>=
          std::ostringstream os;
          os << args[param_i++]->to_int();
          ret += os.str();
          break;
        }
        case '%':
          ret += "%";
          break;
        default:
          throw new ExceptionValue("unknown printf pattern character: %c",
                                   pattern[i]);
          break;
      }
    } else {
      ret += pattern[i];
    }
  }
  return ret;
}

void tora::tora_printf(const std::vector<SharedPtr<Value>> &args) {
  std::cout << tora_sprintf(args);
}
