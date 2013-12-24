#include "tora.h"
#include <vm.h>
#include <util.h>
#include <sys/file.h>
#include <value/array.h>
#include <value/hash.h>
#include <value/exception.h>
#include <value/class.h>

#ifdef _WIN32
#include <winerror.h>
int flock(int fd, int ope) {
    throw new tora::ErrnoExceptionValue(ERROR_CALL_NOT_IMPLEMENTED);
    return -1;
}
#define   LOCK_SH   1
#define   LOCK_EX   2
#define   LOCK_NB   4
#define   LOCK_UN   8
#endif

using namespace tora;

static SharedPtr<Value> Fcntl_flock(VM *vm, Value *fileno_v, Value *operation_v) {
    if (fileno_v->value_type != VALUE_TYPE_INT) {
        throw new ExceptionValue("You passed non-integer value for Fcntl.flock() argument 1 : %s", fileno_v->type_str());
    }
    if (operation_v->value_type != VALUE_TYPE_INT) {
        throw new ExceptionValue("You passed non-integer value for Fcntl.flock() argument 2 : %s", operation_v->type_str());
    }

    if (flock(fileno_v->to_int(), operation_v->to_int()) == 0) {
        return new_undef_value();
    } else {
        throw new ErrnoExceptionValue(get_errno());
    }
}

extern "C" {

TORA_EXPORT
void Init_Fcntl(VM* vm) {
    vm->add_function("flock", new CallbackFunction(Fcntl_flock));
    vm->add_constant("LOCK_SH", LOCK_SH);
    vm->add_constant("LOCK_EX", LOCK_EX);
    vm->add_constant("LOCK_UN", LOCK_UN);
}

}
