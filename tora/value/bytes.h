#ifndef TORA_VALUE_BYTES_H_
#define TORA_VALUE_BYTES_H_

#include "../value.h"

namespace tora {

  typedef std::string BytesImpl;

  class MortalBytesValue : public MortalValue {
    static Value* new_value(const std::string s);
  public:
    MortalBytesValue(const std::string s)
      : MortalValue(new_value(s)) { }
  };

  static BytesImpl* get_bytes_value(const Value* v) {
    assert(type(v) == VALUE_TYPE_BYTES);
    return static_cast<BytesImpl*>(v->ptr_value_);
  }

  tra_int bytes_length(const Value* v);
  std::string bytes_stringify(Value* self);

};

#endif  // TORA_VALUE_BYTES_H_
