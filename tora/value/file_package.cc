#include "file_package.h"

using namespace tora;

Value* tora::file_package_find(Value *v, ID id)
{
  FilePackageImpl* pkg = static_cast<FilePackageImpl*>(get_ptr_value(v));
  auto iter = pkg->data_->find(id);
  if (iter != pkg->data_->end()) {
    return iter->second.get();
  } else {
    return NULL;
  }
}

