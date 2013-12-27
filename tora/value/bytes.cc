#include "bytes.h"
#include "../value.h"

using namespace tora;

Value* MortalBytesValue::new_value(const std::string s)
{
  Value * v = new Value(VALUE_TYPE_BYTES);
  v->ptr_value_ = new BytesImpl(s);
  return v;
}

tra_int tora::bytes_length(const Value* self) {
  return get_bytes_value(self)->size();
}

// Should I validate UTF-8?
std::string tora::bytes_stringify(Value* self)
{
  return *get_bytes_value(self);
}
