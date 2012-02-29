#include "time.h"
#include "../vm.h"
#include "../value/object.h"
#include "../value/symbol.h"
#include "../value/pointer.h"
#include <ctime>
#include <iomanip>
#include <errno.h>

using namespace tora;

/**
 * my $t = Time.new(Int $epoch);
 *
 * Create a time object from $epoch time.
 * Timezone set to local.
 */
static SharedPtr<Value> time_new(VM* vm, Value* klass, Value* t) {
    ObjectValue *o = new ObjectValue(klass->upcast<SymbolValue>()->id, vm);
    SharedPtr<Value> iv = t->to_int();

    struct tm * buf = new tm;
    struct tm * retlocal = localtime_r((const time_t *)&(iv->upcast<IntValue>()->int_value), buf);
    if (!retlocal) {
        delete buf;
        return new ExceptionValue("Error in localtime_r: %s", strerror(errno));
    }
    o->set_value(vm->symbol_table->get_id("t"), new PointerValue(buf));
    return o;
}

/**
 * my $t = Time.now()
 *
 * Get a new instance of time object. It contains current time in local timezone.
 */
static SharedPtr<Value> time_now(VM* vm, Value* klass) {
    std::time_t t = std::time(NULL);
    ObjectValue *o = new ObjectValue(klass->upcast<SymbolValue>()->id, vm);

    struct tm * buf = new tm;
    struct tm * retlocal = localtime_r(&t, buf);
    if (!retlocal) {
        delete buf;
        return new ExceptionValue("Error in localtime_r: %s", strerror(errno));
    }
    o->set_value(vm->symbol_table->get_id("t"), new PointerValue(buf));
    return o;
}

static SharedPtr<Value> time_DESTROY(VM* vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);

    SharedPtr<Value> t = self->upcast<ObjectValue>()->get_value(vm->symbol_table->get_id("t"));
    assert(t->value_type == VALUE_TYPE_POINTER);
    delete t->upcast<PointerValue>()->ptr();
    return UndefValue::instance();
}

/**
 * $time.epoch() : Int
 *
 * Get a epoch time from Time object.
 */
static SharedPtr<Value> time_epoch(VM* vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> t = self->upcast<ObjectValue>()->get_value(vm->symbol_table->get_id("t"));
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
    if (format_s->is_exception()) { return format_s; }

    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> t = self->upcast<ObjectValue>()->get_value(vm->symbol_table->get_id("t"));
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
    pkg->add_method(vm->symbol_table->get_id("now"), new CallbackFunction(time_now));
    pkg->add_method(vm->symbol_table->get_id("DESTROY"), new CallbackFunction(time_DESTROY));
    pkg->add_method(vm->symbol_table->get_id("epoch"), new CallbackFunction(time_epoch));
    pkg->add_method(vm->symbol_table->get_id("strftime"), new CallbackFunction(time_strftime));
}

