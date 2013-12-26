#ifndef TORA_OBJECT_H_
#define TORA_OBJECT_H_

#include "shared_ptr.h"
#include <vector>
#include <memory>
#include <re2/re2.h>

namespace tora {

class VM;
class Value;
class RE2RegexpValue;
class SharedValue;

void Init_Array(VM *vm);
void Init_Caller(VM *vm);
void Init_Code(VM *vm);
void Init_Dir(VM *vm);
void Init_DynaLoader(VM *vm);
void Init_Env(VM *vm);
void Init_File(VM *vm);
void Init_Internals(VM *vm);
void Init_Class(VM *vm);
void Init_MetaClass(VM *vm);
void Init_Object(VM *vm);
void Init_Stat(VM *vm);
void Init_Str(VM *vm);
void Init_Symbol(VM *vm);
void Init_Time(VM *vm);
void Init_Bytes(VM *vm);
void Init_Regexp(VM *vm);
void Init_RE2_Regexp_Matched(VM *vm);
void Init_Hash(VM *vm);
void Init_Int(VM *vm);
void Init_Double(VM *vm);
void Init_FilePackage(VM *vm);

SharedPtr<Value> Symbol_bless(VM *vm, Value *self, Value *data);
SharedValue Dir_new(VM *vm, const std::string &dirname);
SharedPtr<Value> File_open(VM *vm, Value *fname, Value *mode);
SharedPtr<Value> RE2_Regexp_Matched_new(
    VM *vm, RE2RegexpValue *re,
    const std::shared_ptr<std::vector<re2::StringPiece>> &matches);

void load_builtin_objects(VM *vm);
};

#endif  // TORA_OBJECT_H_
