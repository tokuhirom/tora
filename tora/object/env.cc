#ifdef _WIN32
// http://gcc.gnu.org/bugzilla/show_bug.cgi?id=40278
#undef __STRICT_ANSI__
#endif

#include "../object.h"
#include "../vm.h"
#include "../value/class.h"
#include "../symbols.gen.h"

#ifdef _WIN32
static void unsetenv(const char* name) {
    if (!name) return;
    std::string env = name;
    env += "=";
    putenv(env.c_str());
}

static void setenv(const char* name, const char* value, int /* overwrite */) {
    if (!name || !value) return;
    std::string env = name;
    env += "=";
    env += value;
    putenv(env.c_str());
}
#endif

using namespace tora;

/**
 * class Env
 *
 * Environment variable class. This is a singleton class.
 */

// TODO: delete $ENV['PATH']

/**
 * $ENV['hoge'] = 'foo';
 *
 * set a environment variable.
 */
static SharedPtr<Value> env_set(VM * vm, Value* self, Value*k, Value*v) {
    if (v->value_type == VALUE_TYPE_UNDEF) {
        unsetenv(k->to_s()->str_value().c_str());
    } else {
        setenv(
            k->to_s()->str_value().c_str(),
            v->to_s()->str_value().c_str(),
            1 // overwrite
        );
    }
    return UndefValue::instance();
}

/**
 * $ENV['hoge'];
 *
 * get a environment variable.
 */
static SharedPtr<Value> env_get(VM * vm, Value* self, Value*k) {
    char *v = getenv(k->to_s()->str_value().c_str());
    if (v) {
        return new StrValue(v);
    } else {
        return UndefValue::instance();
    }
}

void tora::Init_Env(VM *vm) {
    SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_ENV_CLASS);
    klass->add_method("get", new CallbackFunction(env_get));
    klass->add_method("__getitem__", new CallbackFunction(env_get));
    klass->add_method("__setitem__", new CallbackFunction(env_set));
    vm->add_builtin_class(klass);
}

