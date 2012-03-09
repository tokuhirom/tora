#ifndef TORA_OBJECT_DIR_H_
#define TORA_OBJECT_DIR_H_

#include "../shared_ptr.h"

namespace tora {

class VM;
class ObjectValue;
class StrValue;

ObjectValue* Dir_new(VM *vm, StrValue *dirname);

void Init_Dir(VM *vm);

};

#endif // TORA_OBJECT_DIR_H_
