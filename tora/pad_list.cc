#include "pad_list.h"
#include "frame.h"
#include "vm.h"
#include "peek.h"
#include "package.h"
#include "value/code.h"

using namespace tora;

PadList::PadList(int vars_cnt, PadList *next) : refcnt(0), next_(next) {
// printf("VARS: %d\n", vars_cnt);
   /*
    for (int i=0; i<vars_cnt; i++) {
        SharedPtr<Value> v = UndefValue::instance();
        pad_.push_back(v);
    }
    */
}

static std::string dump_array(VM *vm, PadList *n) {
    Inspector ins(vm);
    std::string o = "[";
    for (auto iter: n->pad_) {
        o += ins.inspect(iter);
    }
    o += "]";
    return o;
}

void PadList::dump(VM *vm) {
    PadList *n = this;
    int i=0;
    while (n) {
        std::clog << i << " " << dump_array(vm, n) << std::endl;
        n = n->next_.get();
        i++;
    }
}

void PadList::set(int i, const SharedPtr<Value> & v) {
    assert(v.get());

    // This retain is required?????????????????????????
    //
    // val->retain();
    //
    //
    //
    //

    if ((int)pad_.size()-1 < i) {
        for (int j=pad_.size()-1; j<i-1; j++) {
            pad_.push_back(UndefValue::instance());
        }
        pad_.insert(pad_.begin()+i, v);
    } else {
        if (1) {
            pad_.erase(pad_.begin()+i);
            pad_.insert(pad_.begin()+i, v);
        } else {
            // fprintf(stderr, "# OK: %zd, %d\n", pad_.size(), i);
            *(pad_.at(i).get()) = *v;
        }
        // pad_->insert(pad_->begin()+i, v);
    }

    assert(pad_.at(i)->value_type == v->value_type);
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

