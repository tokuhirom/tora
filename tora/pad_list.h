#ifndef TORA_PAD_LIST_H_
#define TORA_PAD_LIST_H_

#include "prim.h"
#include "shared_ptr.h"
#include "value/array.h"

namespace tora {

class ArrayValue;
class Value;

class PadList {
private:
    PRIM_DECL(PadList);
    ArrayValue pad_;
    SharedPtr<PadList> next_;
public:
    PadList(int vars_cnt, PadList *next);
    void set(int index, const SharedPtr<Value> & val);
    SharedPtr<Value> get(int index) const;
    PadList *upper(int level) const;
    void dump(VM *vm);
    SharedPtr<Value> get_dynamic(int level, int index) const;
    void set_dynamic(int level, int index, const SharedPtr<Value> & val);
};

};

#endif // TORA_PAD_LIST_H_
