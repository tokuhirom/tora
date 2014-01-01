#ifndef TORA_PAD_LIST_H_
#define TORA_PAD_LIST_H_

#include "shared_ptr.h"
#include <vector>
#include <memory>

namespace tora {

  class SharedValue;
class Value;
class VM;

class PadList {
 public:
  std::vector<SharedValue> pad_;
  std::shared_ptr<PadList>  next_;

 public:
  PadList(int vars_cnt, std::shared_ptr<PadList> next=NULL);
  void set(int index, Value* val);
  SharedValue get(int index) const;
  std::shared_ptr<PadList> upper(int level) const;
  void dump(VM *vm) const;
  SharedValue get_dynamic(int level, int index) const;
  void set_dynamic(int level, int index, Value* val);
  size_t size() const;
};
};

#endif  // TORA_PAD_LIST_H_
