#include "file_package.h"

using namespace tora;

namespace tora {
  class FilePackageImpl {
  public:
    ID name_id_;
    std::shared_ptr<std::map<ID, SharedValue>> data_;

    FilePackageImpl(ID name_id, std::shared_ptr<std::map<ID, SharedValue>> data)
      : name_id_(name_id), data_(data) { }
  };
};

Value* MortalFilePackageValue::new_file_package_value(ID name_id, const std::shared_ptr<std::map<ID, SharedValue>> & src)
{
  Value* v = new Value(VALUE_TYPE_FILE_PACKAGE);
  v->ptr_value_ = new FilePackageImpl(name_id, src);
  return v;
}

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

