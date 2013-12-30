#ifndef TORA_PAD_LIST_H_
#define TORA_PAD_LIST_H_

#include "shared_ptr.h"
#include <vector>
#include <memory>

namespace tora {

class Value;
class VM;

class PadList {
 public:
  std::vector<SharedPtr<Value>> pad_;
  std::shared_ptr<PadList>  next_;

 public:
  PadList(int vars_cnt, std::shared_ptr<PadList> next=NULL);
  void set(int index, const SharedPtr<Value> &val);
  SharedPtr<Value> get(int index) const;
  PadList *upper(int level) const;
  void dump(VM *vm) const;
  SharedPtr<Value> get_dynamic(int level, int index) const;
  void set_dynamic(int level, int index, const SharedPtr<Value> &val);
  size_t size() const;
};
};

#endif  // TORA_PAD_LIST_H_
