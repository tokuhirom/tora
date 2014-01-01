#include "pad_list.h"
#include "frame.h"
#include "vm.h"
#include "peek.h"
#include "value/code.h"
#include <iostream>
#include <vector>

using namespace tora;

PadList::PadList(int vars_cnt, std::shared_ptr<PadList> next) : next_(next) {
  // printf("VARS: %d\n", vars_cnt);
  /*
   for (int i=0; i<vars_cnt; i++) {
       SharedPtr<Value> v = new_undef_value();
       pad_.push_back(v);
   }
   */
}

static std::string dump_array(VM *vm, const PadList *n) {
  Inspector ins(vm);
  std::string o = "[";
  for (auto iter: n->pad_) {
    o += ins.inspect(iter.get());
  }
  o += "]";
  return o;
}

void PadList::dump(VM *vm) const {
  const PadList *n = this;
  int i = 0;
  while (n) {
    std::clog << i << " " << dump_array(vm, n) << std::endl;
    n = n->next_.get();
    i++;
  }
}

size_t PadList::size() const {
  const PadList *n = this;
  size_t i = 0;
  while (n) {
    n = n->next_.get();
    ++i;
  }
  return i;
}

void PadList::set(int i, Value* v) {
  assert(v);

  if ((int)pad_.size() - 1 < i) {
    for (int j = pad_.size() - 1; j < i - 1; j++) {
      MortalValue undef(new_undef_value());
      pad_.push_back(undef);
    }
    pad_.insert(pad_.begin() + i, v);
  } else {
    if (0) {
      pad_.erase(pad_.begin() + i);
      pad_.insert(pad_.begin() + i, v);
    } else {
      // fprintf(stderr, "# OK: %ld, %d\n", (long int) pad_.size(), i);
      pad_.at(i).reset(v);
    }
    // pad_->insert(pad_->begin()+i, v);
  }

  assert(pad_.at(i)->value_type == v->value_type);
}

SharedValue PadList::get_dynamic(int level, int index) const {
  std::shared_ptr<PadList> pl = upper(level);
  assert(pl);
  return pl->get(index);
}

SharedValue PadList::get(int index) const {
  // printf("GET:: %d, %ld\n", index, (long int) pad_.size());
  if (index < pad_.size()) {
    return pad_.at(index);
  } else {
    // printf("UNDEF:: %d, %ld\n", index, (long int) pad_.size());
    MortalValue undef(new_undef_value());
    return undef;
  }
}

void PadList::set_dynamic(int level, int index, Value* val) {
  upper(level)->set(index, val);
}

std::shared_ptr<PadList> PadList::upper(int level) const {
  std::shared_ptr<PadList> n = next_;
  for (int i = 1; i < level; i++) {
    n = n->next_;
  }
  return n;
}
