#ifndef TORA_OBJECT_SYMBOL_T_
#define TORA_OBJECT_SYMBOL_T_

#include "../shared_ptr.h"

namespace tora {

class VM;
class Value;

void Init_Symbol(VM *vm);

SharedPtr<Value> Symbol_bless(VM * vm, Value* self, Value *data);

};

#endif // TORA_OBJECT_SYMBOL_T_
