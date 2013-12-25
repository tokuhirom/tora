#include <ctime>

#include "../object.h"
#include "../vm.h"
#include "../value/object.h"
#include "../symbols.gen.h"

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
  return new ObjectValue(vm, vm->get_builtin_class(SYMBOL_TIME_CLASS).get(),
                         new_ptr_value(buf));
}

static SharedPtr<Value> time_DESTROY(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_OBJECT);

  SharedPtr<Value> t = self->upcast<ObjectValue>()->data();
  assert(t->value_type == VALUE_TYPE_POINTER);
  delete static_cast<struct tm*>(get_ptr_value(t));
  return new_undef_value();
}

/**
 * $time.epoch() : Int
 *
 * Get a epoch time from Time object.
 */
static SharedPtr<Value> time_epoch(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_OBJECT);
  SharedPtr<Value> t = self->upcast<ObjectValue>()->data();
  std::time_t ret = std::mktime(static_cast<struct tm*>(get_ptr_value(t)));
  return new IntValue(ret);
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
  SharedPtr<Value> t = self->upcast<ObjectValue>()->data();
  assert(t->value_type == VALUE_TYPE_POINTER);

  char out[256];

  void* tm = get_ptr_value(t);
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
  SharedPtr<Value> t = self->upcast<ObjectValue>()->data();
  return static_cast<struct tm*>(get_ptr_value(t));
}

/**
 * $time.year() : Int
 *
 * Get a year from $time.
 */
static SharedPtr<Value> time_year(VM* vm, Value* self) {
  return new IntValue(GET_TM(self)->tm_year + 1900);
}

/**
 * $time.month() : Int
 *
 * Get a month from $time.
 */
static SharedPtr<Value> time_month(VM* vm, Value* self) {
  return new IntValue(GET_TM(self)->tm_mon + 1);
}

/**
 * $time.day() : Int
 *
 * Get a month from $time.
 */
static SharedPtr<Value> time_day(VM* vm, Value* self) {
  return new IntValue(GET_TM(self)->tm_mday);
}

/**
 * $time.hour() : Int
 *
 * Get a month from $time.
 */
static SharedPtr<Value> time_hour(VM* vm, Value* self) {
  return new IntValue(GET_TM(self)->tm_hour);
}

/**
 * $time.minute() : Int
 *
 * Get a month value from $time.
 */
static SharedPtr<Value> time_minute(VM* vm, Value* self) {
  return new IntValue(GET_TM(self)->tm_min);
}

/**
 * $time.minute() : Int
 *
 * Get a month value from $time.
 */
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
  int ret = static_cast<struct tm*>(get_ptr_value(t))->tm_wday;
  return new IntValue(ret + 1);
}

void tora::Init_Time(VM* vm) {
  // TODO: strptime
  SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_TIME_CLASS);
  klass->add_method("new", new CallbackFunction(time_new));
  klass->add_method("now", new CallbackFunction(time_new));
  klass->add_method("DESTROY", new CallbackFunction(time_DESTROY));
  klass->add_method("epoch", new CallbackFunction(time_epoch));
  klass->add_method("strftime", new CallbackFunction(time_strftime));
  klass->add_method("month", new CallbackFunction(time_month));
  klass->add_method("year", new CallbackFunction(time_year));
  klass->add_method("day", new CallbackFunction(time_day));
  klass->add_method("hour", new CallbackFunction(time_hour));
  klass->add_method("minute", new CallbackFunction(time_minute));
  klass->add_method("min", new CallbackFunction(time_minute));
  klass->add_method("second", new CallbackFunction(time_second));
  klass->add_method("day_of_week", new CallbackFunction(time_wday));
  vm->add_builtin_class(klass);
}
