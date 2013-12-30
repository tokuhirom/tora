#include <ctime>

#include "../object.h"
#include "../vm.h"
#include "../value/object.h"
#include "../value/pointer.h"
#include "../value/int.h"
#include "../value/exception.h"
#include "../symbols.gen.h"
#include "../class_builder.h"

#ifdef _WIN32
struct tm* localtime_r(const time_t* timer, struct tm* res) {
  struct tm* lt;
  lt = localtime(timer);
  if (lt == NULL || res == NULL) return NULL;
  memcpy(res, lt, sizeof(*res));
  return res;
}
#endif

using namespace tora;

/**
 * class Time
 *
 * The Time object.
 */

/**
 * my $t = Time.new(Int $epoch);
 * my $t = Time.now(Int $epoch);
 *
 * Create a time object from $epoch time.
 * Timezone set to local.
 */
static SharedPtr<Value> time_new(VM* vm,
                                 const std::vector<SharedPtr<Value>>& args) {
  if (!(args.size() == 1 || args.size() == 2)) {
    throw new ExceptionValue(
        "ArgumentException: Time.new([$epoch]) is valid, but you passed %d "
        "arguments.",
        args.size());
  }
  time_t i;
  if (args.size() == 1) {
    i = time(NULL);
  } else {
    const SharedPtr<Value>& t = args.at(1);
    i = t->to_int();
  }

  struct tm* buf = new tm;
  struct tm* retlocal = localtime_r(&i, buf);
  if (!retlocal) {
    delete buf;
    throw new ExceptionValue("Error in localtime_r: %s",
                             get_strerror(get_errno()).c_str());
  }
  MortalPointerValue ptr(buf);
  MortalObjectValue obj(vm, vm->get_builtin_class(SYMBOL_TIME_CLASS).get(), ptr.get());
  return obj.get();
}

static SharedPtr<Value> time_DESTROY(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_OBJECT);

  SharedValue t = object_data(self);
  assert(t->value_type == VALUE_TYPE_POINTER);
  delete static_cast<struct tm*>(get_ptr_value(t.get()));
  return new_undef_value();
}

/**
 * $time.epoch() : Int
 *
 * Get a epoch time from Time object.
 */
static SharedPtr<Value> time_epoch(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_OBJECT);
  SharedValue t = object_data(self);
  std::time_t ret = std::mktime(static_cast<struct tm*>(get_ptr_value(t.get())));
  MortalIntValue i(ret);
  return i.get();
}

/**
 * $time.strftime(Str $format) : Str
 *
 * Format time object to string by $format. See strftime(3).
 *
 * Limitation: Result string must be smaller than 256-1 bytes.(patches welcome)
 */
static SharedPtr<Value> time_strftime(VM* vm, Value* self, Value* format) {
  std::string format_s = format->to_s();

  assert(self->value_type == VALUE_TYPE_OBJECT);
  SharedValue t = object_data(self);
  assert(t->value_type == VALUE_TYPE_POINTER);

  char out[256];

  void* tm = get_ptr_value(t.get());
  size_t r = strftime(out, sizeof(out), format_s.c_str(),
                      static_cast<const struct tm*>(tm));
  if (r == sizeof(out)) {
    // should be retry?
    return new ExceptionValue("strftime overflow: %ld", (long int)sizeof(r));
  }

  return new_str_value(out);
}

static struct tm* GET_TM(Value* self) {
  assert(self->value_type == VALUE_TYPE_OBJECT);
  SharedValue t = object_data(self);
  return static_cast<struct tm*>(get_ptr_value(t.get()));
}

/**
 * $time.year() : Int
 *
 * Get a year from $time.
 */
static SharedPtr<Value> time_year(VM* vm, Value* self) {
  MortalIntValue i(GET_TM(self)->tm_year + 1900);
  return i.get();
}

/**
 * $time.month() : Int
 *
 * Get a month from $time.
 */
static SharedPtr<Value> time_month(VM* vm, Value* self) {
  MortalIntValue i(GET_TM(self)->tm_mon + 1);
  return i.get();
}

/**
 * $time.day() : Int
 *
 * Get a month from $time.
 */
static SharedPtr<Value> time_day(VM* vm, Value* self) {
  MortalIntValue miv(GET_TM(self)->tm_mday); return miv.get();
}

/**
 * $time.hour() : Int
 *
 * Get a month from $time.
 */
static SharedPtr<Value> time_hour(VM* vm, Value* self) {
  MortalIntValue miv(GET_TM(self)->tm_hour); return miv.get();
}

/**
 * $time.minute() : Int
 *
 * Get a month value from $time.
 */
static SharedPtr<Value> time_minute(VM* vm, Value* self) {
  MortalIntValue miv(GET_TM(self)->tm_min); return miv.get();
}

/**
 * $time.minute() : Int
 *
 * Get a month value from $time.
 */
static SharedPtr<Value> time_second(VM* vm, Value* self) {
  MortalIntValue miv(GET_TM(self)->tm_sec); return miv.get();
}

/**
 * $time.day_of_week() : Int
 *
 * This method returns day of week. It returns 1..7.
 */
static SharedPtr<Value> time_wday(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_OBJECT);
  SharedValue t = object_data(self);
  int ret = static_cast<struct tm*>(get_ptr_value(t.get()))->tm_wday;
  MortalIntValue miv(ret + 1); return miv.get();
}

void tora::Init_Time(VM* vm) {
  // TODO: strptime
  ClassBuilder builder(vm, SYMBOL_TIME_CLASS);
  builder.add_method("new", std::make_shared<CallbackFunction>(time_new));
  builder.add_method("now", std::make_shared<CallbackFunction>(time_new));
  builder.add_method("DESTROY", std::make_shared<CallbackFunction>(time_DESTROY));
  builder.add_method("epoch", std::make_shared<CallbackFunction>(time_epoch));
  builder.add_method("strftime", std::make_shared<CallbackFunction>(time_strftime));
  builder.add_method("month", std::make_shared<CallbackFunction>(time_month));
  builder.add_method("year", std::make_shared<CallbackFunction>(time_year));
  builder.add_method("day", std::make_shared<CallbackFunction>(time_day));
  builder.add_method("hour", std::make_shared<CallbackFunction>(time_hour));
  builder.add_method("minute", std::make_shared<CallbackFunction>(time_minute));
  builder.add_method("min", std::make_shared<CallbackFunction>(time_minute));
  builder.add_method("second", std::make_shared<CallbackFunction>(time_second));
  builder.add_method("day_of_week", std::make_shared<CallbackFunction>(time_wday));
  vm->add_builtin_class(builder.value());
}
