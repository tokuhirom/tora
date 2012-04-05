#include "tora.h"
#include <vm.h>
#include <util.h>
#include <sys/file.h>
#include <value/array.h>
#include <value/hash.h>
#include <value/exception.h>
#include <package.h>

using namespace tora;

static SharedPtr<Value> Fcntl_flock(VM *vm, Value *str, Value *fileno_v, Value *operation_v) {
    if (fileno_v->value_type != VALUE_TYPE_INT) {
        throw new ExceptionValue("You passed non-integer value for Fcntl.flock() argument 1 : %s", fileno_v->type_str());
    }
    if (operation_v->value_type != VALUE_TYPE_INT) {
        throw new ExceptionValue("You passed non-integer value for Fcntl.flock() argument 2 : %s", operation_v->type_str());
    }

    if (flock(fileno_v->to_int(), operation_v->to_int()) == 0) {
        return UndefValue::instance();
    } else {
        throw new ErrnoExceptionValue(get_errno());
    }
}

extern "C" {

TORA_EXPORT
void Init_Fcntl(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("Fcntl");
    pkg->add_method("flock", new CallbackFunction(Fcntl_flock));
    pkg->add_constant("LOCK_SH", LOCK_SH);
    pkg->add_constant("LOCK_EX", LOCK_EX);
    pkg->add_constant("LOCK_UN", LOCK_UN);
}

}
