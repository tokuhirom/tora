#ifndef TORA_INSPECTOR_H_
#define TORA_INSPECTOR_H_

#include "shared_ptr.h"
#include <string>

namespace tora {

class VM;
class Value;

/**
 * convert value to eval-able string.
 *
 * This class is similar to Data::Dumper in Perl5.
 */
class Inspector {
 private:
  VM *vm_;

 public:
  Inspector(VM *vm);

  std::string inspect(const SharedPtr<Value> &v) const {
    return inspect(v.get());
  }
  std::string inspect(Value *v) const;
};
};

#endif  // TORA_INSPECTOR_H_
