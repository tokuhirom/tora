#include "../vm.h"
#include "socket.h"
#include "../value/object.h"
#include <sys/types.h>
#include <sys/socket.h>

using namespace tora;

// TODO: remove from core. split it to ext/Socket/

/**
 * class Socket
 *
 * This is a low level socket class.
 */


/**
 * Socket.socket(Int domain, Int type, Int protocol);
 * Socket.socket(Int domain, Int type);
 *
 */
static SharedPtr<Value> sock_socket(VM * vm, Value* klass, Value* domain_v, Value* type_v, Value* protocol_v) {
    SharedPtr<Value> domain_i = domain_v->to_int();
    if (domain_i->value_type == VALUE_TYPE_EXCEPTION) { return domain_i; }
    int domain = domain_i->upcast<IntValue>()->int_value;

    SharedPtr<Value> type_i = type_v->to_int();
    if (type_i->value_type == VALUE_TYPE_EXCEPTION) { return type_i; }
    int type = type_i->upcast<IntValue>()->int_value;
#ifdef SOCK_CLOEXEC
    type |= SOCK_CLOEXEC;
#endif

    SharedPtr<Value> protocol_i = protocol_v->to_int();
    if (protocol_i->value_type == VALUE_TYPE_EXCEPTION) { return protocol_i; }
    int protocol = protocol_i->upcast<IntValue>()->int_value;

    int sock = socket(domain, type, protocol);
    if (sock == -1) {
        return new ExceptionValue(errno);
    } else {
        ObjectValue * ov = new ObjectValue(vm->symbol_table->get_id("Socket::Socket"), vm);
        ov->set_value(vm->symbol_table->get_id("_fd"), new IntValue(sock));

        return ov;
    }
}

static SharedPtr<Value> sock_sock_DESTROY(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> v = self->upcast<ObjectValue>()->get_value(vm->symbol_table->get_id("_fd"));
    if (v.get()) { v.get()->upcast<IntValue>(); }
    return UndefValue::instance();
}

void tora::Init_Socket(VM* vm) {
    // socket, bind, listen, accept, send, recv
    // "accept", "bind", "connect", "getpeername", "getsockname", "getsockopt", "listen", "recv", "send", "setsockopt",
    // "shutdown", "socket", "socketpair"

    {
        SharedPtr<Package> pkg = vm->find_package("Socket");
        pkg->add_method(vm->symbol_table->get_id("socket"), new CallbackFunction(sock_socket));
    }

    {
        SharedPtr<Package> pkg = vm->find_package("Socket::Socket");
        pkg->add_method(vm->symbol_table->get_id("DESTROY"), new CallbackFunction(sock_sock_DESTROY));
    }
}

