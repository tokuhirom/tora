#include "tora.h"
#include <vm.h>
#include <value/object.h>
#include <value/pointer.h>
#include <value/class.h>
#include <value/code.h>
#include <shared_ptr.h>

#include <uv.h>

using namespace tora;

struct _uv_data {
    VM* vm;
    uv_loop_t* loop;
    void* data;
    SharedPtr<CodeValue> connect_cb;
    SharedPtr<CodeValue> connection_cb;
    SharedPtr<CodeValue> write_cb;
    SharedPtr<CodeValue> read_cb;
    SharedPtr<CodeValue> close_cb;
    SharedPtr<CodeValue> timer_cb;
    SharedPtr<CodeValue> idle_cb;
    _uv_data(VM* vm, uv_loop_t* loop, void* data) {
        this->vm = vm;
        this->loop = loop;
        this->data = data;
    }
};

static void __uv_close_cb(uv_handle_t* handle) {
    VM *vm = ((_uv_data*) handle->data)->vm;
    SharedPtr<CodeValue> callback = ((_uv_data*) handle->data)->close_cb;
    if (callback != NULL) {
        vm->function_call_ex(0, callback, UndefValue::instance());
    }
}

static SharedPtr<Value> _uv_close(VM *vm, Value *self, Value *callback_v) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> handle_ = self->upcast<ObjectValue>()->data();
    assert(handle_->value_type == VALUE_TYPE_POINTER);
    uv_handle_t *handle = (uv_handle_t*) handle_->upcast<PointerValue>()->ptr();

    if (callback_v->value_type != VALUE_TYPE_CODE) {
        ((_uv_data*) handle->data)->close_cb = NULL;
    } else {
        ((_uv_data*) handle->data)->close_cb = callback_v->upcast<CodeValue>();
    }

    uv_close(handle, __uv_close_cb);
    return UndefValue::instance();
}

static SharedPtr<Value> _uv_DESTROY(VM *vm, Value *self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> handle_ = self->upcast<ObjectValue>()->data();
    assert(handle_->value_type == VALUE_TYPE_POINTER);
    uv_handle_t *handle = (uv_handle_t*) handle_->upcast<PointerValue>()->ptr();
    uv_close(handle, NULL);
    if (handle->data) delete ((_uv_data*) handle->data);
    delete handle;
    return UndefValue::instance();
}

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
    return UndefValue::instance();
}

static SharedPtr<Value> _uv_loop_unref(VM * vm, Value* self) {
    uv_loop_t* loop = static_cast<uv_loop_t*>(self->upcast<ObjectValue>()->data()->upcast<PointerValue>()->ptr());
    uv_unref(loop);
    return UndefValue::instance();
}

static SharedPtr<Value> _uv_timer_new(VM * vm, const std::vector<SharedPtr<Value>>& args) {
    SharedPtr<Value> self = args.at(0);
    uv_loop_t* loop;
    if (args.size() == 1) {
        loop = uv_default_loop();
    } else {
        loop = static_cast<uv_loop_t*>(args.at(2)->upcast<ObjectValue>()->data()->upcast<PointerValue>()->ptr());
    }
    uv_timer_t* timer = new uv_timer_t;
    if (uv_timer_init(loop, timer) != 0) {
        delete timer;
        throw new ExceptionValue(uv_strerror(uv_last_error(loop)));
    } else {
        timer->data = new _uv_data(vm, loop, NULL);
        return new ObjectValue(vm, vm->symbol_table->get_id("UV::Timer"), new PointerValue(timer));
    }
}

void __uv_timer_cb(uv_timer_t* timer, int status) {
    VM *vm = ((_uv_data*) timer->data)->vm;
    SharedPtr<CodeValue> callback = ((_uv_data*) timer->data)->timer_cb;
    if (callback != NULL) {
        vm->stack.push_back(new IntValue(status));
        vm->function_call_ex(1, callback, UndefValue::instance());
    }
}

static SharedPtr<Value> _uv_timer_start(VM * vm, Value* self, Value* callback_v, Value* timeout_v, Value* repeat_v) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> timer_ = self->upcast<ObjectValue>()->data();
    assert(timer_->value_type == VALUE_TYPE_POINTER);
    uv_timer_t *timer = (uv_timer_t*) timer_->upcast<PointerValue>()->ptr();

    if (callback_v->value_type != VALUE_TYPE_CODE) {
        ((_uv_data*) timer->data)->timer_cb = NULL;
    } else {
        ((_uv_data*) timer->data)->timer_cb = callback_v->upcast<CodeValue>();
    }
    assert(timeout_v->value_type == VALUE_TYPE_INT);
    assert(repeat_v->value_type == VALUE_TYPE_INT);

    if (uv_timer_start(timer, __uv_timer_cb, timeout_v->to_int(), repeat_v->to_int()) != 0) {
        throw new ExceptionValue(uv_strerror(uv_last_error(((_uv_data*) timer->data)->loop)));
    }
    return UndefValue::instance();
}

static SharedPtr<Value> _uv_timer_stop(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> timer_ = self->upcast<ObjectValue>()->data();
    assert(timer_->value_type == VALUE_TYPE_POINTER);
    uv_timer_t *timer = (uv_timer_t*) timer_->upcast<PointerValue>()->ptr();

    if (uv_timer_stop(timer) != 0) {
        throw new ExceptionValue(uv_strerror(uv_last_error(((_uv_data*) timer->data)->loop)));
    }
    return UndefValue::instance();
}

static SharedPtr<Value> _uv_tcp_new(VM * vm, const std::vector<SharedPtr<Value>>& args) {
    SharedPtr<Value> self = args.at(0);
    uv_loop_t* loop;
    if (args.size() == 1) {
        loop = uv_default_loop();
    } else {
        loop = static_cast<uv_loop_t*>(args.at(2)->upcast<ObjectValue>()->data()->upcast<PointerValue>()->ptr());
    }
    uv_tcp_t* tcp = new uv_tcp_t;
    if (uv_tcp_init(loop, tcp) != 0) {
        delete tcp;
        throw new ExceptionValue(uv_strerror(uv_last_error(loop)));
    } else {
        tcp->data = new _uv_data(vm, loop, NULL);
        return new ObjectValue(vm, vm->symbol_table->get_id("UV::TCP"), new PointerValue(tcp));
    }
}

static SharedPtr<Value> _uv_tcp_nodelay(VM * vm, Value* self, Value* enable_v) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> tcp_ = self->upcast<ObjectValue>()->data();
    assert(tcp_->value_type == VALUE_TYPE_POINTER);
    uv_tcp_t *tcp = (uv_tcp_t*) tcp_->upcast<PointerValue>()->ptr();

    assert(enable_v->value_type == VALUE_TYPE_BOOL);

    if (uv_tcp_nodelay(tcp, enable_v->to_bool()) != 0) {
        throw new ExceptionValue(uv_strerror(uv_last_error(((_uv_data*) tcp->data)->loop)));
    }
    return UndefValue::instance();
}

static void __uv_tcp_connect_cb(uv_connect_t* req, int status) {
    VM *vm = ((_uv_data*) req->handle->data)->vm;
    SharedPtr<CodeValue> callback = ((_uv_data*) req->handle->data)->connect_cb;
    if (callback != NULL) {
        vm->stack.push_back(new IntValue(status));
        vm->function_call_ex(1, callback, UndefValue::instance());
    }
}

static SharedPtr<Value> _uv_tcp_connect(VM * vm, Value* self, Value* addr_v, Value* callback_v) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> tcp_ = self->upcast<ObjectValue>()->data();
    assert(tcp_->value_type == VALUE_TYPE_POINTER);
    uv_tcp_t *tcp = (uv_tcp_t*) tcp_->upcast<PointerValue>()->ptr();

    SharedPtr<Value> addr = addr_v->to_s();
    const std::string & addr_s = addr_v->upcast<StrValue>()->str_value();
    if (callback_v->value_type != VALUE_TYPE_CODE) {
        ((_uv_data*) tcp->data)->connect_cb = NULL;
    } else {
        ((_uv_data*) tcp->data)->connect_cb = callback_v->upcast<CodeValue>();
    }

    static uv_connect_t req;
    if (uv_tcp_connect(&req, tcp, *(const sockaddr_in*)addr_s.c_str(), __uv_tcp_connect_cb) != 0) {
        throw new ExceptionValue(uv_strerror(uv_last_error(((_uv_data*) tcp->data)->loop)));
    }
    return UndefValue::instance();
}

static void __uv_write_cb(uv_write_t* req, int status) {
    VM *vm = ((_uv_data*) req->handle->data)->vm;
    SharedPtr<CodeValue> callback = ((_uv_data*) req->handle->data)->write_cb;
    if (callback != NULL) {
        vm->stack.push_back(new IntValue(status));
        vm->function_call_ex(1, callback, UndefValue::instance());
    }
}

static SharedPtr<Value> _uv_write(VM * vm, Value* self, Value* src_v, Value* callback_v) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> tcp_ = self->upcast<ObjectValue>()->data();
    assert(tcp_->value_type == VALUE_TYPE_POINTER);
    uv_tcp_t *tcp = (uv_tcp_t*) tcp_->upcast<PointerValue>()->ptr();

    SharedPtr<Value> src = src_v->to_s();
    const std::string &s = src->upcast<StrValue>()->str_value();
    assert(callback_v->value_type == VALUE_TYPE_CODE);
    if (callback_v->value_type != VALUE_TYPE_CODE) {
        ((_uv_data*) tcp->data)->write_cb = NULL;
    } else {
        ((_uv_data*) tcp->data)->write_cb = callback_v->upcast<CodeValue>();
    }

    static uv_write_t req;
    static uv_buf_t buf = uv_buf_init((char*) s.c_str(), s.size());
    if (uv_write(&req, (uv_stream_t*) tcp, &buf, 1, __uv_write_cb) != 0) {
        throw new ExceptionValue(uv_strerror(uv_last_error(((_uv_data*) tcp->data)->loop)));
    }
    return UndefValue::instance();
}

static uv_buf_t __uv_alloc_cb(uv_handle_t* handle, size_t suggested_size) {
    return uv_buf_init(new char[suggested_size], suggested_size);
}

static void __uv_read_cb(uv_stream_t* handle, ssize_t nread, uv_buf_t buf) {
    VM *vm = ((_uv_data*) handle->data)->vm;
    SharedPtr<CodeValue> callback = ((_uv_data*) handle->data)->read_cb;
    if (callback != NULL) {
        if (nread >= 0)
            vm->stack.push_back(new BytesValue(buf.base, nread));
        else
            vm->stack.push_back(UndefValue::instance());
        vm->function_call_ex(1, callback, UndefValue::instance());
    }
}

static SharedPtr<Value> _uv_read(VM * vm, Value* self, Value* callback_v) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> tcp_ = self->upcast<ObjectValue>()->data();
    assert(tcp_->value_type == VALUE_TYPE_POINTER);
    uv_tcp_t *tcp = (uv_tcp_t*) tcp_->upcast<PointerValue>()->ptr();

    if (callback_v->value_type != VALUE_TYPE_CODE) {
        ((_uv_data*) tcp->data)->read_cb = NULL;
    } else {
        ((_uv_data*) tcp->data)->read_cb = callback_v->upcast<CodeValue>();
    }

    if (uv_read_start((uv_stream_t*) tcp, __uv_alloc_cb, __uv_read_cb) != 0) {
        throw new ExceptionValue(uv_strerror(uv_last_error(((_uv_data*) tcp->data)->loop)));
    }
    return UndefValue::instance();
}

static SharedPtr<Value> _uv_tcp_bind(VM * vm, Value* self, Value* addr_v) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> tcp_ = self->upcast<ObjectValue>()->data();
    assert(tcp_->value_type == VALUE_TYPE_POINTER);
    uv_tcp_t *tcp = (uv_tcp_t*) tcp_->upcast<PointerValue>()->ptr();

    SharedPtr<Value> addr = addr_v->to_s();
    const std::string & addr_s = addr_v->upcast<StrValue>()->str_value();

    if (uv_tcp_bind(tcp, *(const sockaddr_in*)addr_s.c_str()) != 0) {
        throw new ExceptionValue(uv_strerror(uv_last_error(((_uv_data*) tcp->data)->loop)));
    }
    return UndefValue::instance();
}

static void __uv_connection_cb(uv_stream_t* handle, int status) {
    VM *vm = ((_uv_data*) handle->data)->vm;
    SharedPtr<CodeValue> callback = ((_uv_data*) handle->data)->connection_cb;
    if (callback != NULL) {
        vm->stack.push_back(new IntValue(status));
        vm->function_call_ex(1, callback, UndefValue::instance());
    }
}

static SharedPtr<Value> _uv_listen(VM * vm, Value* self, Value* backlog_v, Value* callback_v) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> tcp_ = self->upcast<ObjectValue>()->data();
    assert(tcp_->value_type == VALUE_TYPE_POINTER);
    uv_tcp_t *tcp = (uv_tcp_t*) tcp_->upcast<PointerValue>()->ptr();

    assert(backlog_v->value_type == VALUE_TYPE_INT);
    if (callback_v->value_type != VALUE_TYPE_CODE) {
        ((_uv_data*) tcp->data)->connection_cb = NULL;
    } else {
        ((_uv_data*) tcp->data)->connection_cb = callback_v->upcast<CodeValue>();
    }

    if (uv_listen((uv_stream_t*) tcp, backlog_v->to_int(), __uv_connection_cb) != 0) {
        throw new ExceptionValue(uv_strerror(uv_last_error(((_uv_data*) tcp->data)->loop)));
    }
    return UndefValue::instance();
}

static SharedPtr<Value> _uv_accept(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> tcp_ = self->upcast<ObjectValue>()->data();
    assert(tcp_->value_type == VALUE_TYPE_POINTER);
    uv_tcp_t *tcp = (uv_tcp_t*) tcp_->upcast<PointerValue>()->ptr();

    uv_loop_t* loop = ((_uv_data*) tcp->data)->loop;
    
    uv_tcp_t* client = new uv_tcp_t;
    if (uv_tcp_init(loop, client) != 0) {
        delete client;
        throw new ExceptionValue(uv_strerror(uv_last_error(loop)));
    }
    if (uv_accept((uv_stream_t*) tcp, (uv_stream_t*) client) != 0) {
        delete client;
        throw new ExceptionValue(uv_strerror(uv_last_error(loop)));
    } else {
        client->data = new _uv_data(vm, loop, NULL);
        return new ObjectValue(vm, vm->symbol_table->get_id("UV::TCP"), new PointerValue(client));
    }
}

static SharedPtr<Value> _uv_idle_new(VM * vm, const std::vector<SharedPtr<Value>>& args) {
    SharedPtr<Value> self = args.at(0);
    uv_loop_t* loop;
    if (args.size() == 1) {
        loop = uv_default_loop();
    } else {
        loop = static_cast<uv_loop_t*>(args.at(2)->upcast<ObjectValue>()->data()->upcast<PointerValue>()->ptr());
    }
    uv_idle_t* idle = new uv_idle_t;
    if (uv_idle_init(loop, idle) != 0) {
        delete idle;
        throw new ExceptionValue(uv_strerror(uv_last_error(loop)));
    } else {
        idle->data = new _uv_data(vm, loop, NULL);
        return new ObjectValue(vm, vm->symbol_table->get_id("UV::Idle"), new PointerValue(idle));
    }
}

void __uv_idle_cb(uv_idle_t* idle, int status) {
    VM *vm = ((_uv_data*) idle->data)->vm;
    SharedPtr<CodeValue> callback = ((_uv_data*) idle->data)->idle_cb;
    if (callback != NULL) {
        vm->stack.push_back(new IntValue(status));
        vm->function_call_ex(1, callback, UndefValue::instance());
    }
}

static SharedPtr<Value> _uv_idle_start(VM * vm, Value* self, Value* callback_v) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> idle_ = self->upcast<ObjectValue>()->data();
    assert(idle_->value_type == VALUE_TYPE_POINTER);
    uv_idle_t *idle = (uv_idle_t*) idle_->upcast<PointerValue>()->ptr();

    if (callback_v->value_type != VALUE_TYPE_CODE) {
        ((_uv_data*) idle->data)->idle_cb = NULL;
    } else {
        ((_uv_data*) idle->data)->idle_cb = callback_v->upcast<CodeValue>();
    }

    if (uv_idle_start(idle, __uv_idle_cb) != 0) {
        throw new ExceptionValue(uv_strerror(uv_last_error(((_uv_data*) idle->data)->loop)));
    }
    return UndefValue::instance();
}

static SharedPtr<Value> _uv_idle_stop(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> idle_ = self->upcast<ObjectValue>()->data();
    assert(idle_->value_type == VALUE_TYPE_POINTER);
    uv_idle_t *idle = (uv_idle_t*) idle_->upcast<PointerValue>()->ptr();

    if (uv_idle_stop(idle) != 0) {
        throw new ExceptionValue(uv_strerror(uv_last_error(((_uv_data*) idle->data)->loop)));
    }
    return UndefValue::instance();
}

extern "C" {

TORA_EXPORT
void Init_UV_(VM* vm) {
    {
        SharedPtr<ClassValue> klass = new ClassValue(vm, "UV");
        klass->add_method("run", new CallbackFunction(_uv_run));
        klass->add_method("default_loop", new CallbackFunction(_uv_default_loop));
        vm->add_class(klass);
    }
    {
        SharedPtr<ClassValue> klass = new ClassValue(vm, "UV::Loop");
        klass->add_method("run", new CallbackFunction(_uv_loop_run));
        klass->add_method("run_once", new CallbackFunction(_uv_loop_run_once));
        klass->add_method("ref", new CallbackFunction(_uv_loop_ref));
        klass->add_method("unref", new CallbackFunction(_uv_loop_unref));
        vm->add_class(klass);
    }
    {
        SharedPtr<ClassValue> klass = new ClassValue(vm, "UV::Timer");
        klass->add_method("new", new CallbackFunction(_uv_timer_new));
        klass->add_method("start", new CallbackFunction(_uv_timer_start));
        klass->add_method("stop", new CallbackFunction(_uv_timer_stop));
        klass->add_method("close", new CallbackFunction(_uv_close));
        klass->add_method("DESTROY", new CallbackFunction(_uv_DESTROY));
        vm->add_class(klass);
    }
    {
        SharedPtr<ClassValue> klass = new ClassValue(vm, "UV::TCP");
        klass->add_method("new", new CallbackFunction(_uv_tcp_new));
        klass->add_method("nodelay", new CallbackFunction(_uv_tcp_nodelay));
        klass->add_method("connect", new CallbackFunction(_uv_tcp_connect));
        klass->add_method("write", new CallbackFunction(_uv_write));
        klass->add_method("read", new CallbackFunction(_uv_read));
        klass->add_method("close", new CallbackFunction(_uv_close));
        klass->add_method("bind", new CallbackFunction(_uv_tcp_bind));
        klass->add_method("listen", new CallbackFunction(_uv_listen));
        klass->add_method("accept", new CallbackFunction(_uv_accept));
        klass->add_method("DESTROY", new CallbackFunction(_uv_DESTROY));
        vm->add_class(klass);
    }
    {
        SharedPtr<ClassValue> klass = new ClassValue(vm, "UV::Idle");
        klass->add_method("new", new CallbackFunction(_uv_idle_new));
        klass->add_method("start", new CallbackFunction(_uv_idle_start));
        klass->add_method("stop", new CallbackFunction(_uv_idle_stop));
        klass->add_method("close", new CallbackFunction(_uv_close));
        klass->add_method("DESTROY", new CallbackFunction(_uv_DESTROY));
        vm->add_class(klass);
    }
}

}
