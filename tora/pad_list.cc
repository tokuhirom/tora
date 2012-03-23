#include "pad_list.h"
#include "frame.h"
#include "vm.h"
#include "peek.h"
#include "package.h"
#include "value/code.h"

using namespace tora;

PadList::PadList(int vars_cnt, PadList *next) : refcnt(0), next_(next) {
// printf("VARS: %d\n", vars_cnt);
    for (int i=0; i<vars_cnt; i++) {
        SharedPtr<Value> v = UndefValue::instance();
        pad_.push_back(v);
    }
}

void PadList::dump(VM *vm) {
    Inspector ins(vm);
    PadList *n = this;
    int i=0;
    while (n) {
        std::clog << i << " " << ins.inspect(&(n->pad_)) << std::endl;
        n = n->next_.get();
        i++;
    }
}

void PadList::set(int index, const SharedPtr<Value> & val) {
    assert(val.get());
    val->retain();
    pad_.set_item(index, val);
    assert(pad_.at(index)->value_type == val->value_type);
}

SharedPtr<Value> PadList::get_dynamic(int level, int index) const {
    PadList * pl = upper(level);
    assert(pl);
    return pl->get(index);
}

SharedPtr<Value> PadList::get(int index) const {
// printf("GET:: %d, %zd\n", index, pad_.size());
    if (index < pad_.size()) {
        return pad_.at(index);
    } else {
// printf("UNDEF:: %d, %zd\n", index, pad_.size());
        return UndefValue::instance();
    }
}

void PadList::set_dynamic(int level, int index, const SharedPtr<Value> & val) {
    upper(level)->set(index, val);
}

PadList *PadList::upper(int level) const {
    PadList * n = next_.get();
    for (int i=1; i<level; i++) {
        n = n->next_.get();
    }
    return n;
}

