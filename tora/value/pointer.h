#ifndef TORA_VALUE_POINTER_H_
#define TORA_VALUE_POINTER_H_

#include "../value.h"

namespace tora {

  class MortalPointerValue : public MortalValue {
    static Value* new_value(void* p) {
      Value* v = new Value(VALUE_TYPE_POINTER);
      v->ptr_value_ = p;
      return v;
    }
  public:
    MortalPointerValue(void* p) : MortalValue(new_value(p)) {
    }
  };
};

#endif
