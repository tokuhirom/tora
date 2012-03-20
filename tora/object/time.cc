#include <ctime>
#include <iomanip>
#include <errno.h>

#include "../object.h"
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
static SharedPtr<Value> time_new(VM* vm, const std::vector<SharedPtr<Value>> &args) {
    if (!(args.size() == 1 || args.size() == 2)) {
        throw new ExceptionValue("ArgumentException: Time.new([$epoch]) is valid, but you passed %d arguments.", args.size());
    }
    const SharedPtr<Value> & klass = args.at(0);
    time_t i;
    if (args.size() == 1) {
        i = time(NULL);
    } else {
        const SharedPtr<Value> & t = args.at(1);
        i = t->to_int();
    }

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

static struct tm* GET_TM(Value *self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> t = self->upcast<ObjectValue>()->data();
    return static_cast<struct tm*>(t->upcast<PointerValue>()->ptr());
}

/**
 * Time.year()
 */
static SharedPtr<Value> time_year(VM* vm, Value* self) {
    return new IntValue(GET_TM(self)->tm_year + 1900);
}

static SharedPtr<Value> time_month(VM* vm, Value* self) {
    return new IntValue(GET_TM(self)->tm_mon + 1);
}

static SharedPtr<Value> time_day(VM* vm, Value* self) {
    return new IntValue(GET_TM(self)->tm_mday);
}

static SharedPtr<Value> time_hour(VM* vm, Value* self) {
    return new IntValue(GET_TM(self)->tm_hour);
}

static SharedPtr<Value> time_minute(VM* vm, Value* self) {
    return new IntValue(GET_TM(self)->tm_min);
}

static SharedPtr<Value> time_second(VM* vm, Value* self) {
    return new IntValue(GET_TM(self)->tm_sec);
}

/**
 * $time.day_of_week() : Int
 *
 * This method returns day of week. It returns 1..7.
 */
static SharedPtr<Value> time_wday(VM* vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> t = self->upcast<ObjectValue>()->data();
    int ret = static_cast<struct tm*>(t->upcast<PointerValue>()->ptr())->tm_wday;
    return new IntValue(ret + 1);
}

void tora::Init_Time(VM *vm) {
    // TODO: strptime
    SharedPtr<Package> pkg = vm->find_package("Time");
    pkg->add_method(vm->symbol_table->get_id("new"), new CallbackFunction(time_new));
    pkg->add_method(vm->symbol_table->get_id("now"), new CallbackFunction(time_new));
    pkg->add_method(vm->symbol_table->get_id("DESTROY"), new CallbackFunction(time_DESTROY));
    pkg->add_method(vm->symbol_table->get_id("epoch"), new CallbackFunction(time_epoch));
    pkg->add_method(vm->symbol_table->get_id("strftime"), new CallbackFunction(time_strftime));
    pkg->add_method(vm->symbol_table->get_id("month"), new CallbackFunction(time_month));
    pkg->add_method(vm->symbol_table->get_id("year"), new CallbackFunction(time_year));
    pkg->add_method(vm->symbol_table->get_id("day"), new CallbackFunction(time_day));
    pkg->add_method(vm->symbol_table->get_id("hour"), new CallbackFunction(time_hour));
    pkg->add_method(vm->symbol_table->get_id("minute"), new CallbackFunction(time_minute));
    pkg->add_method(vm->symbol_table->get_id("min"), new CallbackFunction(time_minute));
    pkg->add_method(vm->symbol_table->get_id("second"), new CallbackFunction(time_second));
    pkg->add_method(vm->symbol_table->get_id("day_of_week"), new CallbackFunction(time_wday));
}

