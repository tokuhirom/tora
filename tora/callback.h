#ifndef TORA_CALLBACK_H_
#define TORA_CALLBACK_H_

#include "shared_ptr.h"

namespace tora {

struct CallbackFunction {
    typedef enum {
        type_vmv = -1,
        type_vm0 = -2,
        type_vm1 = -3,
        type_vm2 = -4,
        type_vm3 = -5,
        type_vm4 = -6,
        type_const_int = 1,
    } callback_type_t;
    typedef SharedPtr<Value> (*func_vmv_t)(VM *, const std::vector<SharedPtr<Value>>&);
    typedef SharedPtr<Value> (*func_vm0_t)(VM *);
    typedef SharedPtr<Value> (*func_vm1_t)(VM *, Value*);
    typedef SharedPtr<Value> (*func_vm2_t)(VM *, Value*, Value*);
    typedef SharedPtr<Value> (*func_vm3_t)(VM *, Value*, Value*, Value*);
    typedef SharedPtr<Value> (*func_vm4_t)(VM *, Value*, Value*, Value*, Value*);
    union {
        func_vmv_t func_vmv;
        func_vm0_t func_vm0;
        func_vm1_t func_vm1;
        func_vm2_t func_vm2;
        func_vm3_t func_vm3;
        func_vm4_t func_vm4;
        int const_int;
    };
    int argc;
    CallbackFunction(func_vmv_t func_) : argc(type_vmv) { func_vmv = func_; }
    CallbackFunction(func_vm0_t func_) : argc(type_vm0) { func_vm0 = func_; }
    CallbackFunction(func_vm1_t func_) : argc(type_vm1) { func_vm1 = func_; }
    CallbackFunction(func_vm2_t func_) : argc(type_vm2) { func_vm2 = func_; }
    CallbackFunction(func_vm3_t func_) : argc(type_vm3) { func_vm3 = func_; }
    CallbackFunction(func_vm4_t func_) : argc(type_vm4) { func_vm4 = func_; }
    CallbackFunction(int n)            : argc(type_const_int) { const_int = n; }
};

};

#endif // TORA_CALLBACK_H_
