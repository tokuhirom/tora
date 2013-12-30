#include "class_builder.h"
#include "vm.h"

using namespace tora;

void tora::ClassBuilder::add_method(const char *name, const std::shared_ptr<CallbackFunction>& func)
{
  ID name_id = vm_->symbol_table->get_id(name);
  tora::class_add_method(mcv_.get(), name_id, func);
}

void tora::ClassBuilder::add_constant(const char *name, tra_int i)
{
  ID name_id = vm_->symbol_table->get_id(name);
  tora::class_add_constant(mcv_.get(), name_id, i);
}

