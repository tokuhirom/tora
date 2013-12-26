#include "../object.h"
#include "../vm.h"
#include "../symbols.gen.h"
#include "../value/class.h"
#include "../class_builder.h"

void tora::Init_Double(VM* vm) {
  ClassBuilder builder(vm, SYMBOL_DOUBLE_CLASS);
  vm->add_builtin_class(builder.value());
}
