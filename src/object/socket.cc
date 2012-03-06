#include "../vm.h"
#include "socket.h"
#include "../value/object.h"
#include "../value/array.h"
#include "../value/tuple.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <boost/scoped_array.hpp>

using namespace tora;

// TODO: remove from core. split it to ext/Socket/

/**
 * class Socket
 *
 * This is a low level socket class.
 */

const int GETFD(VM *vm, Value * self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> fd = self->upcast<ObjectValue>()->get_value(vm->symbol_table->get_id("_fd"));
    return fd->value_type == VALUE_TYPE_INT ? fd->upcast<IntValue>()->int_value : -1;
}

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

static SharedPtr<Value> sock_sock_connect(VM * vm, Value* self, Value*addr_v) {
    SharedPtr<Value> addr = addr_v->to_s();
    if (addr->is_exception()) { return addr; }

    const std::string & addr_s = addr_v->upcast<StrValue>()->str_value;
    int ret = connect(GETFD(vm, self), (const sockaddr*)addr_s.c_str(), addr_s.size());
    if (ret==0) {
        return UndefValue::instance();
    } else {
        return new ExceptionValue(errno);
    }
}

/**
 * Socket.sockaddr_in(port, value)
 *
 * Return sockaddr struct in bytes.
 */
static SharedPtr<Value> sock_sockaddr_in(VM * vm, Value*klass, Value* port, Value* host) {
    // TODO: return bytes.
    SharedPtr<Value> port_i = port->to_int();
    if (port_i->is_exception()) { return port_i; }

    SharedPtr<Value> host_s = host->to_s();
    if (host_s->is_exception()) { return host_s; }
    const std::string &ip = host_s->upcast<StrValue>()->str_value;

    struct in_addr addr;
    if (ip.size() == sizeof(addr) || ip.size() == 4) {
        addr.s_addr =
            (ip.at(0) & 0xFF) << 24 |
            (ip.at(1) & 0xFF) << 16 |
            (ip.at(2) & 0xFF) <<  8 |
            (ip.at(3) & 0xFF);
    } else {
        return new ExceptionValue("Bad arg length for %s, length is %d, should be %d", "Socket.sockaddr_in", ip.size(), sizeof(addr));
    }

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port_i->upcast<IntValue>()->int_value);
    sin.sin_addr.s_addr = htonl(addr.s_addr);
#  ifdef HAS_SOCKADDR_SA_LEN
    sin.sin_len = sizeof (sin);
#  endif
    std::string ret((const char*)&sin, sizeof(sin));
    return new StrValue(ret);
}

/**
 * Socket.inet_aton($ip);
 * Socket.inet_aton($hostname);
 *
 * This function calls inet_aton(3) or gethostbyname(3).
 */
static SharedPtr<Value> sock_inet_aton(VM * vm, Value*klass, Value* host_v) {
    struct in_addr ip_address;
    SharedPtr<Value> host = host_v->to_s();
    if (host->is_exception()) { return host; }

    const std::string & host_s = host->upcast<StrValue>()->str_value;
    if ((host_s.at(0) != '\0') && inet_aton(host_s.c_str(), &ip_address)) {
        std::string ret((const char*)&ip_address, sizeof(ip_address));
        return new StrValue(ret);
    } else {
        // TODO: gethostbyname_r?
        struct hostent *phe = gethostbyname(host_s.c_str());
        if (phe && phe->h_addrtype == AF_INET && phe->h_length == 4) {
            std::string ret((const char*)phe->h_addr, phe->h_length);
            return new StrValue(ret);
        } else {
            return UndefValue::instance();
        }
    }
}

static SharedPtr<Value> sock_write(VM * vm, Value*self, Value* src_v) {
    SharedPtr<Value> src = src_v->to_s();
    if (src->is_exception()) { return src; }

    const std::string &s = src->upcast<StrValue>()->str_value;
    int fd = GETFD(vm, self);
    int ret = write(fd, s.c_str(), s.size());
    if (ret == -1) {
        return new ExceptionValue(errno);
    } else {
        return new IntValue(ret);
    }
}

/**
 * $sock.read()
 */
static SharedPtr<Value> sock_read(VM * vm, const std::vector<SharedPtr<Value>>&args) {
    SharedPtr<Value> self = args.at(0);
    int fd = GETFD(vm, self.get());
    if (args.size() == 1) {
        int readed = 0;
        std::string bbuf;
        while (1) {
            char buf[4096];
            int ret = read(fd, buf, sizeof(buf));
            if (ret > 0) {
                printf("CONT!\n");
                readed += ret;
                bbuf += std::string(buf, ret);
                continue;
            } else if (ret == 0) { // EOF
                return new StrValue(bbuf);
            } else {
                return new ExceptionValue(errno);
            }
        }
    } else if (args.size() == 2) {
        SharedPtr<Value> size_v = args.at(1)->to_int();
        if (size_v->is_exception()) { return size_v; }

        int size = size_v->upcast<IntValue>()->int_value;
        boost::scoped_array<char> buf(new char[size]);
#ifndef NDEBUG
        memset(buf.get(), 0, size);
#endif
        int ret = read(fd, buf.get(), size);
        if (ret >= 0) {
            SharedPtr<Value> s = new StrValue(std::string(buf.get(), ret));
            return s;
        } else {
            return new ExceptionValue(errno);
        }
    } else {
        return new ExceptionValue("Invalid argument count for Socket::Socket::read: %zd", args.size());
    }
}

static SharedPtr<Value> sock_sock_bind(VM * vm, Value* self, Value*addr_v) {
    SharedPtr<Value> addr = addr_v->to_s();
    if (addr->is_exception()) { return addr; }

    const std::string & addr_s = addr_v->upcast<StrValue>()->str_value;
    int ret = bind(GETFD(vm, self), (const sockaddr*)addr_s.c_str(), addr_s.size());
    if (ret==0) {
        return UndefValue::instance();
    } else {
        return new ExceptionValue(errno);
    }
}

static SharedPtr<Value> sock_sock_listen(VM * vm, Value* self, Value* queue_v) {
    SharedPtr<Value> queue_i = queue_v->to_int();
    if (queue_i->is_exception()) { return queue_i; }

    int ret = listen(GETFD(vm, self), queue_i->upcast<IntValue>()->int_value);
    if (ret == 0) {
        return UndefValue::instance();
    } else {
        return new ExceptionValue(errno);
    }
}

static SharedPtr<Value> sock_sock_setsockopt(VM * vm, Value* self, Value* level_v, Value* optname_v, Value *optval_v) {
    SharedPtr<Value> level_i = level_v->to_int();
    if (level_i->is_exception()) { return level_i; }

    SharedPtr<Value> optname_i = optname_v->to_int();
    if (optname_i->is_exception()) { return optname_i; }

    const void *optval;
    socklen_t optlen;
    int n;
    if (optval_v->value_type == VALUE_TYPE_INT) {
        n = optval_v->upcast<IntValue>()->int_value;
        optval = &n;
        optlen = sizeof(n);
    } else {
        SharedPtr<Value> optval_s = optval_v->to_s();
        if (optval_s->is_exception()) { return optval_s; }
        optval = optval_s->upcast<StrValue>()->str_value.c_str();
        optlen = optval_s->upcast<StrValue>()->str_value.size();
    }

    int ret = setsockopt(GETFD(vm, self), level_i->upcast<IntValue>()->int_value, optname_i->upcast<IntValue>()->int_value, optval, optlen);
    if (ret == 0) {
        return UndefValue::instance();
    } else {
        return new ExceptionValue(errno);
    }
}

static SharedPtr<Value> sock_sock_close(VM * vm, Value* self) {
    int ret = close(GETFD(vm, self));
    if (ret == 0) {
        return UndefValue::instance();
    } else {
        return new ExceptionValue(errno);
    }
}

/**
 * $socket.accept()
 */
static SharedPtr<Value> sock_sock_accept(VM * vm, Value* self) {
    char namebuf[MAXPATHLEN];
    socklen_t len = sizeof namebuf;

    int fd = accept(GETFD(vm, self), (struct sockaddr *)namebuf, &len);
    if (fd >= 0) {
        ObjectValue *new_sock = new ObjectValue(vm->symbol_table->get_id("Socket::Socket"), vm);
        new_sock->set_value(vm->symbol_table->get_id("_fd"), new IntValue(fd));
        SharedPtr<TupleValue> t = new TupleValue();
        t->push(new StrValue(std::string(namebuf, len)));
        t->push(new_sock);
        return t;
    } else {
        return new ExceptionValue(errno);
    }
}

void tora::Init_Socket(VM* vm) {
    // socket, bind, listen, accept, send, recv
    // "accept", "bind", "connect", "getpeername", "getsockname", "getsockopt", "listen", "recv", "send", "setsockopt",
    // "shutdown", "socket", "socketpair"

    {
        SharedPtr<Package> pkg = vm->find_package("Socket");
        pkg->add_method(vm->symbol_table->get_id("socket"), new CallbackFunction(sock_socket));
        pkg->add_method(vm->symbol_table->get_id("sockaddr_in"), new CallbackFunction(sock_sockaddr_in));
        pkg->add_method(vm->symbol_table->get_id("inet_aton"), new CallbackFunction(sock_inet_aton));
    }

    {
        SharedPtr<Package> pkg = vm->find_package("Socket::Socket");
        pkg->add_method(vm->symbol_table->get_id("connect"), new CallbackFunction(sock_sock_connect));
        pkg->add_method(vm->symbol_table->get_id("write"), new CallbackFunction(sock_write));
        pkg->add_method(vm->symbol_table->get_id("read"), new CallbackFunction(sock_read));
        pkg->add_method(vm->symbol_table->get_id("DESTROY"), new CallbackFunction(sock_sock_DESTROY));
        pkg->add_method(vm->symbol_table->get_id("bind"), new CallbackFunction(sock_sock_bind));
        pkg->add_method(vm->symbol_table->get_id("listen"), new CallbackFunction(sock_sock_listen));
        pkg->add_method(vm->symbol_table->get_id("close"), new CallbackFunction(sock_sock_close));
        pkg->add_method(vm->symbol_table->get_id("accept"), new CallbackFunction(sock_sock_accept));
        pkg->add_method(vm->symbol_table->get_id("setsockopt"), new CallbackFunction(sock_sock_setsockopt));
    }
}

