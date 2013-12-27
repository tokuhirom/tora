#ifndef TORA_FILE_H_
#define TORA_FILE_H_

#include "../value.h"
#include <cstdio>

namespace tora {

  class MortalFileValue : public MortalValue {
  public:
    explicit MortalFileValue(FILE *fp)
      : MortalValue(new Value(VALUE_TYPE_FILE, static_cast<void*>(fp))) { }
  };

  static FILE * get_file_pointer(Value* v) {
    assert(type(v) == VALUE_TYPE_FILE);
    return static_cast<FILE*>(get_ptr_value(v));
  }

  void file_close(Value* self);
  std::string file_slurp(Value* self);

};

#endif  // TORA_FILE_H_
