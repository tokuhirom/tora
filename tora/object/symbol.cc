#include <stdlib.h>

#include "../object.h"
#include "../vm.h"
#include "../symbols.gen.h"
#include "../value/object.h"
#include "../value/symbol.h"
#include "../value/class.h"
#include "../class_builder.h"

using namespace tora;

/**
 * class Symbol
 *
 * The symbol class.
 */

/**
 * $symbol.bless($data)
 *
 * Create a new object from $symbol and $data.
 */
SharedPtr<Value> tora::Symbol_bless(VM *vm, Value *klass, Value *data) {
  assert(klass->value_type == VALUE_TYPE_SYMBOL);
  MortalObjectValue o(
      vm,
      vm->get_class(symbol_id(klass)).get(),
      data
  );
  return o.get();
}

void tora::Init_Symbol(VM *vm) {
  ClassBuilder builder(vm, SYMBOL_SYMBOL_CLASS);
  builder.add_method("bless", std::make_shared<CallbackFunction>(Symbol_bless));
  vm->add_builtin_class(builder.value());
}
