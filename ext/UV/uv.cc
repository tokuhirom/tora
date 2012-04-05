#include "tora.h"
#include <vm.h>
#include <value/object.h>
#include <value/pointer.h>
#include <value/code.h>
#include <package.h>
#include <shared_ptr.h>

#include <uv.h>

using namespace tora;

struct _uv_data {
    VM* vm;
    uv_loop_t* loop;
    void* data;
    SharedPtr<CodeValue> callback;
    _uv_data(VM* vm, uv_loop_t* loop, void* data) {
        this->vm = vm;
        this->loop = loop;
        this->data = data;
    }
};

static SharedPtr<Value> _uv_default_loop(VM * vm, Value* self) {
    return new ObjectValue(vm, vm->symbol_table->get_id("UV::Loop"), new PointerValue(uv_default_loop()));
}

static SharedPtr<Value> _uv_run(VM * vm, Value* self) {
    return new IntValue(uv_run(uv_default_loop()));
}

static SharedPtr<Value> _uv_loop_run(VM * vm, Value* self) {
    uv_loop_t* loop = static_cast<uv_loop_t*>(self->upcast<ObjectValue>()->data()->upcast<PointerValue>()->ptr());
    return new IntValue(uv_run(loop));
}

static SharedPtr<Value> _uv_loop_run_once(VM * vm, Value* self) {
    uv_loop_t* loop = static_cast<uv_loop_t*>(self->upcast<ObjectValue>()->data()->upcast<PointerValue>()->ptr());
    return new IntValue(uv_run_once(loop));
}

static SharedPtr<Value> _uv_loop_ref(VM * vm, Value* self) {
    uv_loop_t* loop = static_cast<uv_loop_t*>(self->upcast<ObjectValue>()->data()->upcast<PointerValue>()->ptr());
    uv_ref(loop);
    return NULL;
}

static SharedPtr<Value> _uv_loop_unref(VM * vm, Value* self) {
    uv_loop_t* loop = static_cast<uv_loop_t*>(self->upcast<ObjectValue>()->data()->upcast<PointerValue>()->ptr());
    uv_unref(loop);
    return NULL;
}

static SharedPtr<Value> _uv_timer_new(VM * vm, const std::vector<SharedPtr<Value>>& args) {
    SharedPtr<Value> self = args.at(0);
    uv_loop_t* loop;
    if (args.size() == 1) {
        loop = uv_default_loop();
    } else {
        // uv.cc:61:48: warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]
        loop = (uv_loop_t*) args.at(2)->to_int();
    }
    uv_timer_t* timer = new uv_timer_t;
    if (uv_timer_init(loop, timer) == 0) {
        timer->data = new _uv_data(vm, loop, NULL);
        return new ObjectValue(vm, vm->symbol_table->get_id("UV::Timer"), new PointerValue(timer));
    } else {
        delete timer;
        throw new ExceptionValue(uv_strerror(uv_last_error(loop)));
    }
}

void __uv_timer_cb(uv_timer_t* timer, int status) {
    VM *vm = ((_uv_data*) timer->data)->vm;
    vm->function_call_ex(
        0, ((_uv_data*) timer->data)->callback, new IntValue(status));
}

static SharedPtr<Value> _uv_timer_start(VM * vm, Value* self, Value* callback_v, Value* timeout_v, Value* repeat_v) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> timer_ = self->upcast<ObjectValue>()->data();
    assert(timer_->value_type == VALUE_TYPE_POINTER);
    uv_timer_t *timer = (uv_timer_t*) timer_->upcast<PointerValue>()->ptr();

    assert(callback_v->value_type == VALUE_TYPE_CODE);
    ((_uv_data*) timer->data)->callback = callback_v->upcast<CodeValue>();
    ((_uv_data*) timer->data)->callback->retain();

    if (uv_timer_start(timer, __uv_timer_cb, timeout_v->to_int(), repeat_v->to_int()) != 0) {
        throw new ExceptionValue(uv_strerror(uv_last_error(((_uv_data*) timer->data)->loop)));
    }
    return UndefValue::instance();
}

static SharedPtr<Value> _uv_timer_DESTROY(VM *vm, Value *self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> timer_ = self->upcast<ObjectValue>()->data();
    assert(timer_->value_type == VALUE_TYPE_INT);
    uv_timer_t *timer = (uv_timer_t*) timer_->upcast<IntValue>()->int_value();
    uv_close((uv_handle_t*) timer, NULL);
    if (timer->data) delete ((_uv_data*) timer->data);
    delete timer;
    return UndefValue::instance();
}

extern "C" {

TORA_EXPORT
void Init_UV_(VM* vm) {
    {
        SharedPtr<Package> pkg = vm->find_package("UV");
        pkg->add_method(vm->symbol_table->get_id("run"), new CallbackFunction(_uv_run));
        pkg->add_method(vm->symbol_table->get_id("default_loop"), new CallbackFunction(_uv_default_loop));
    }
    {
        SharedPtr<Package> pkg = vm->find_package("UV::Loop");
        pkg->add_method(vm->symbol_table->get_id("run"), new CallbackFunction(_uv_loop_run));
        pkg->add_method(vm->symbol_table->get_id("run_once"), new CallbackFunction(_uv_loop_run_once));
        pkg->add_method(vm->symbol_table->get_id("ref"), new CallbackFunction(_uv_loop_ref));
        pkg->add_method(vm->symbol_table->get_id("unref"), new CallbackFunction(_uv_loop_unref));
    }
    {
        SharedPtr<Package> pkg = vm->find_package("UV::Timer");
        pkg->add_method(vm->symbol_table->get_id("new"), new CallbackFunction(_uv_timer_new));
        pkg->add_method(vm->symbol_table->get_id("start"), new CallbackFunction(_uv_timer_start));
        pkg->add_method(vm->symbol_table->get_id("DESTROY"), new CallbackFunction(_uv_timer_DESTROY));
    }
}

}
