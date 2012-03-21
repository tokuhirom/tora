#ifndef TORA_OBJECT_H_
#define TORA_OBJECT_H_

#include "shared_ptr.h"
#include <vector>

namespace tora {

class VM;
class ObjectValue;
class StrValue;
class Value;

void Init_Array(VM *vm);
void Init_Caller(VM *vm);
void Init_Code(VM *vm);
void Init_Dir(VM *vm);
void Init_DynaLoader(VM *vm);
void Init_Env(VM *vm);
void Init_File(VM *vm);
void Init_Internals(VM *vm);
void Init_MetaClass(VM *vm);
void Init_Object(VM *vm);
void Init_Socket(VM *vm);
void Init_Stat(VM *vm);
void Init_Str(VM *vm);
void Init_Symbol(VM *vm);
void Init_Time(VM *vm);
void Init_Bytes(VM *vm);
void Init_Regexp(VM *vm);

SharedPtr<Value> Symbol_bless(VM * vm, Value* self, Value *data);
ObjectValue* Dir_new(VM *vm, StrValue *dirname);
SharedPtr<Value> File_open(VM *vm, Value *fname, Value *mode);

void load_builtin_objects(VM *vm);

};

#endif // TORA_OBJECT_H_
