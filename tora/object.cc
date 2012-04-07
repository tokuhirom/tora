#include "object.h"

using namespace tora;

void tora::load_builtin_objects(VM *vm) {
    // language core
    Init_Array(vm);
    Init_Str(vm);
    Init_Caller(vm);
    Init_Code(vm);
    Init_Symbol(vm);
    Init_Object(vm);
    Init_Bytes(vm);
    Init_Regexp(vm);
    Init_RE2_Regexp_Matched(vm);
    Init_Hash(vm);
    Init_Int(vm);
    Init_Double(vm);
    Init_FilePackage(vm);

    // OOP
    Init_Class(vm);
    Init_MetaClass(vm);

    // DynaLoader
    Init_DynaLoader(vm);

    // OS
    Init_Dir(vm);
    Init_Env(vm);
    Init_Stat(vm);
    Init_File(vm);
    Init_Time(vm);

    // misc
    Init_Internals(vm);
}

