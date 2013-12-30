#ifndef TORA_DISASM_H_
#define TORA_DISASM_H_

#include "op.h"
#include "op_array.h"

namespace tora {

class Disasm {
 public:
  static void disasm_op(OP* op);

  static void disasm(const std::shared_ptr<OPArray>& ops);
};
};

#endif  // TORA_DISASM_H_
