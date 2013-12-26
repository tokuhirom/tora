#ifndef TORA_CLASS_BUILDER_H_
#define TORA_CLASS_BUILDER_H_

#include "value/class.h"

namespace tora {
  class VM;

  class ClassBuilder {
    VM* vm_;
    MortalClassValue mcv_;
  public:
    ClassBuilder(VM* vm, ID name_id)
      : vm_(vm), mcv_(vm, name_id) {
    }
    void add_method(const char* name, const CallbackFunction* func);
    void add_constant(const char* name, tra_int i);
    Value* value() {
      return mcv_.get();
    }
  };
};

#endif // TORA_CLASS_BUILDER_H_
