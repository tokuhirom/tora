#include "../object.h"
#include "../vm.h"
#include "../symbols.gen.h"
#include "../value/class.h"

void tora::Init_Int(VM* vm) {
    SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_INT_CLASS);
    vm->add_builtin_class(klass);
}
