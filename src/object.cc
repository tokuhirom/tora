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
    Init_MetaClass(vm);
    Init_Bytes(vm);

    // DynaLoader
    Init_DynaLoader(vm);

    // OS
    Init_Dir(vm);
    Init_Env(vm);
    Init_Stat(vm);
    Init_Socket(vm);
    Init_File(vm);
    Init_Time(vm);

    // misc
    Init_Internals(vm);
}

