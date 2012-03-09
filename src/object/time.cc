#include <ctime>
#include <iomanip>
#include <errno.h>

#include "time.h"
#include "../vm.h"
#include "../package.h"
#include "../value/object.h"
#include "../value/symbol.h"
#include "../value/pointer.h"

using namespace tora;

/**
 * my $t = Time.new(Int $epoch);
 * my $t = Time.now(Int $epoch);
 *
 * Create a time object from $epoch time.
 * Timezone set to local.
 */
static SharedPtr<Value> time_new(VM* vm, Value* klass, Value* t) {
    time_t i = t->to_int();

    struct tm * buf = new tm;
    struct tm * retlocal = localtime_r(&i, buf);
    if (!retlocal) {
        delete buf;
        throw new ExceptionValue("Error in localtime_r: %s", strerror(errno));
    }
    return new ObjectValue(vm, klass->upcast<SymbolValue>()->id, new PointerValue(buf));
}

static SharedPtr<Value> time_DESTROY(VM* vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);

    SharedPtr<Value> t = self->upcast<ObjectValue>()->data();
    assert(t->value_type == VALUE_TYPE_POINTER);
    delete static_cast<struct tm*>(t->upcast<PointerValue>()->ptr());
    return UndefValue::instance();
}

/**
 * $time.epoch() : Int
 *
 * Get a epoch time from Time object.
 */
static SharedPtr<Value> time_epoch(VM* vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> t = self->upcast<ObjectValue>()->data();
    std::time_t ret = std::mktime(static_cast<struct tm*>(t->upcast<PointerValue>()->ptr()));
    return new IntValue(ret);
}

/**
 * $time.strftime(Str $format) : Str
 *
 * Format time object to string by $format. See strftime(3).
 *
 * Limitation: Result string must be smaller than 256-1 bytes.(patches welcome)
 */
static SharedPtr<Value> time_strftime(VM* vm, Value* self, Value *format) {
    SharedPtr<Value> format_s = format->to_s();

    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> t = self->upcast<ObjectValue>()->data();
    assert(t->value_type == VALUE_TYPE_POINTER);

    char out[256];

    void * tm = t->upcast<PointerValue>()->ptr();
    size_t r = strftime(out, sizeof(out), format_s->upcast<StrValue>()->str_value.c_str(), static_cast<const struct tm*>(tm));
    if (r == sizeof(out)) {
        // should be retry?
        return new ExceptionValue("strftime overflow: %d", sizeof(r));
    }

    return new StrValue(out);
}

void tora::Init_Time(VM *vm) {
    // TODO: month, year, day, hour, minute, second, day_of_month, day_of_week, tzoffset, strptime
    SharedPtr<Package> pkg = vm->find_package("Time");
    pkg->add_method(vm->symbol_table->get_id("new"), new CallbackFunction(time_new));
    pkg->add_method(vm->symbol_table->get_id("now"), new CallbackFunction(time_new));
    pkg->add_method(vm->symbol_table->get_id("DESTROY"), new CallbackFunction(time_DESTROY));
    pkg->add_method(vm->symbol_table->get_id("epoch"), new CallbackFunction(time_epoch));
    pkg->add_method(vm->symbol_table->get_id("strftime"), new CallbackFunction(time_strftime));
}

