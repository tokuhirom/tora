#include <stddef.h>

#include "tora.h"
#include <vm.h>
#include <value/array.h>
#include <value/hash.h>
#include <value/tuple.h>
#include <value/pointer.h>
#include <value/object.h>
#include <value/code.h>
#include <package.h>
#include <shared_ptr.h>

#include <boost/scoped_array.hpp>

#include "mongoose/mongoose.h"

#define MAX_HEADER_NAME_LEN 1024
#define MAX_HEADERS         128

using namespace tora;

static struct mg_context *SELF(Value *self) {
    const SharedPtr<Value> data = self->upcast<ObjectValue>()->data();
    if (data->value_type == VALUE_TYPE_UNDEF) {
        throw new ExceptionValue("You cannot call this method without mg_context.");
    } else {
        return (struct mg_context*)data->upcast<PointerValue>()->ptr();
    }
}

static inline struct mg_connection *SELF_CONN(Value *self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    assert(self->upcast<ObjectValue>()->data()->value_type == VALUE_TYPE_POINTER);
    return (struct mg_connection*)self->upcast<ObjectValue>()->data()->upcast<PointerValue>()->ptr();
}

static void * cb(
    enum mg_event event,
    struct mg_connection *conn,
    const struct mg_request_info *request_info
) {
    ObjectValue * self = (ObjectValue*)request_info->user_data;
    if (self->data()->upcast<ArrayValue>()->size() == 2) {
        VM *vm = self->vm();

        CodeValue* code = static_cast<CodeValue*>(self->data()->upcast<ArrayValue>()->at(1).get());
        SharedPtr<HashValue> req(new HashValue());

        SharedPtr<Value> conn_v = new ObjectValue(vm, vm->symbol_table->get_id("HTTP::Server::Mongoose::Connection"), new PointerValue(conn));

        // $httpd.set_callback(-> $event, $connection, $request_info {});
        int argcnt = 3;
        vm->stack.push_back(req);
        vm->stack.push_back(conn_v);
        vm->stack.push_back(new IntValue(event));

        vm->function_call_ex(argcnt, code, UndefValue::instance());
        const SharedPtr<Value> ret = vm->stack.back();
        return ret->to_bool() ? (void*)"" /* mark processed. */ : NULL;
    }
    return NULL;
}

/**
 * HTTP::Server::Mongoose.new()
 *
 * Get a new instance of Mongoose.
 */
static SharedPtr<Value> HTTP_Server_Mongoose_new(VM *vm, Value *self) {
    return new ObjectValue(vm, vm->symbol_table->get_id("HTTP::Server::Mongoose"), new ArrayValue());
}

/**
 * $httpd.start()
 *
 * Start the server.
 */
// struct mg_context *mg_start(mg_callback_t callback, void *user_data,
//                             const char **options);
static SharedPtr<Value> HTTP_Server_Mongoose_start(VM *vm, Value *self, Value *options_v) {
    if (options_v->value_type != VALUE_TYPE_HASH) {
        throw new ExceptionValue("Second argument for HTTP::Server::Mongoose::start must be Hash, but you passed %s", options_v->type_str());
    }
    HashValue* options_hv = options_v->upcast<HashValue>();
    boost::scoped_array<const char *> options(new const char*[options_hv->size()*2+1]);
    int i=0;
    for (auto iter=options_hv->begin(); iter!=options_hv->end(); iter++) {
        options[i++] = iter->first.c_str();
        options[i++] = iter->second->to_s()->str_value().c_str();
    }
    options[i++] = NULL;
    struct mg_context * ctx = mg_start(
        cb, (void*)self, options.get()
    );
    self->upcast<ObjectValue>()->data()->upcast<ArrayValue>()->set_item(0, new PointerValue(ctx));
    return UndefValue::instance();
}

/**
 * $httpd.stop(); : Undef
 *
 * Stop the server.
 * Returns Undefined value.
 */
static SharedPtr<Value> HTTP_Server_Mongoose_stop(VM *vm, Value *self) {
    mg_stop(SELF(self));
    return UndefValue::instance();
}

// const char *mg_get_option(const struct mg_context *ctx, const char *name);
/**
 * $httpd.get_option(Str $name) : Str
 *
 * Get the option from $httpd.
 */
static SharedPtr<Value> HTTP_Server_Mongoose_get_option(VM *vm, Value *self, Value *name_v) {
    return new StrValue(mg_get_option(SELF(self), name_v->to_s()->c_str()));
}

// const char **mg_get_valid_option_names(void);
/**
 * $httpd.get_valid_option_names() : Array[Str]
 *
 * Get valid option names in Array[Str].
 */
static SharedPtr<Value> HTTP_Server_Mongoose_get_valid_option_names(VM *vm, Value *self) {
    const char **names = mg_get_valid_option_names();
    SharedPtr<ArrayValue> av(new ArrayValue());
    while (*names) {
        av->push_back(new StrValue(*names));
    }
    return av;
}

// int mg_modify_passwords_file(const char *passwords_file_name,
//                              const char *domain,
//                              const char *user,
//                              const char *password);
// int mg_printf(struct mg_connection *, const char *fmt, ...)

// int mg_write(struct mg_connection *, const void *buf, size_t len);
static SharedPtr<Value> HTTP_Server_Mongoose_write(VM *vm, Value *self, Value *buf_v) {
    const std::string & buf = buf_v->to_s()->str_value();
    struct mg_connection *conn = SELF_CONN(self);
    return new IntValue(mg_write(conn, buf.c_str(), buf.size()));
}

// void mg_send_file(struct mg_connection *conn, const char *path);
static SharedPtr<Value> HTTP_Server_Mongoose_send_file(VM *vm, Value *self, Value *path_v) {
    mg_send_file(SELF_CONN(self), path_v->to_s()->str_value().c_str());
    return UndefValue::instance();
}


// int mg_read(struct mg_connection *, void *buf, size_t len);
// const char *mg_get_header(const struct mg_connection *, const char *name);

// int mg_get_var(const char *data, size_t data_len,
//                const char *var_name, char *buf, size_t buf_len);
// int mg_get_cookie(const struct mg_connection *,
//                   const char *cookie_name, char *buf, size_t buf_len);

// const char *mg_version(void);
static SharedPtr<Value> HTTP_Server_Mongoose_get_version(VM *vm, Value *self) {
    return new StrValue(mg_version());
}

extern "C" {

void Init_HTTP_Server_Mongoose(VM* vm) {
    {
        SharedPtr<Package> pkg = vm->find_package("HTTP::Server::Mongoose");
        pkg->add_method("new", new CallbackFunction(HTTP_Server_Mongoose_new));
        pkg->add_method("start", new CallbackFunction(HTTP_Server_Mongoose_start));
        pkg->add_method("stop", new CallbackFunction(HTTP_Server_Mongoose_stop));
        pkg->add_method("get_option", new CallbackFunction(HTTP_Server_Mongoose_get_option));
        pkg->add_method("get_valid_option_names", new CallbackFunction(HTTP_Server_Mongoose_get_valid_option_names));
        pkg->add_method("get_version", new CallbackFunction(HTTP_Server_Mongoose_get_version));

        pkg->add_constant("MG_NEW_REQUEST",      MG_NEW_REQUEST);
        pkg->add_constant("MG_HTTP_ERROR",       MG_HTTP_ERROR);
        pkg->add_constant("MG_EVENT_LOG",        MG_EVENT_LOG);
        pkg->add_constant("MG_INIT_SSL",         MG_INIT_SSL);
        pkg->add_constant("MG_REQUEST_COMPLETE", MG_REQUEST_COMPLETE);
    }

    {
        SharedPtr<Package> pkg = vm->find_package("HTTP::Server::Mongoose::Connection");
        pkg->add_method("send_file", new CallbackFunction(HTTP_Server_Mongoose_send_file));
        pkg->add_method("write", new CallbackFunction(HTTP_Server_Mongoose_write));
    }
}

}
