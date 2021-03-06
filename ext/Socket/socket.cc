#include "object.h"
#include "vm.h"
#include "value/object.h"
#include "value/array.h"
#include "value/tuple.h"
#include "value/bytes.h"
#include "value/class.h"

#include <sys/types.h>
#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <boost/scoped_array.hpp>

#ifdef _WIN32
static int inet_aton(const char *cp, struct in_addr *addr) {
    addr->s_addr = inet_addr(cp);
    return (addr->s_addr == INADDR_NONE) ? 0 : 1;
}
#endif

using namespace tora;

// TODO: remove from core. split it to ext/Socket/

/**
 * class Socket
 *
 * This is a low level socket class.
 */

const int GETFD(VM *vm, Value * self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> fd = self->upcast<ObjectValue>()->data();
    return fd->value_type == VALUE_TYPE_INT ? fd->upcast<IntValue>()->int_value() : -1;
}

/**
 * Socket.socket(Int domain, Int type, Int protocol);
 * Socket.socket(Int domain, Int type);
 *
 */
static SharedPtr<Value> sock_socket(VM * vm, Value* klass, Value* domain_v, Value* type_v, Value* protocol_v) {
    int domain = domain_v->to_int();

    int type = type_v->to_int();
#ifdef SOCK_CLOEXEC
    type |= SOCK_CLOEXEC;
#endif

    int protocol = protocol_v->to_int();

    int sock = socket(domain, type, protocol);
    if (sock == -1) {
        throw new ErrnoExceptionValue(get_errno());
    } else {
        return new ObjectValue(vm, vm->get_class(vm->get_id("Socket::Socket")), new IntValue(sock));
    }
}

static SharedPtr<Value> sock_sock_DESTROY(VM * vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_OBJECT);
    SharedPtr<Value> v = self->upcast<ObjectValue>()->data();
    if (v.get()) { v.get()->upcast<IntValue>(); }
    return UndefValue::instance();
}

static SharedPtr<Value> sock_sock_connect(VM * vm, Value* self, Value*addr_v) {
    SharedPtr<Value> addr = addr_v->to_s();
    if (addr->is_exception()) { return addr; }

    const std::string & addr_s = addr_v->upcast<StrValue>()->str_value();
    int ret = connect(GETFD(vm, self), (const sockaddr*)addr_s.c_str(), addr_s.size());
    if (ret==0) {
        return UndefValue::instance();
    } else {
        return new ErrnoExceptionValue(get_errno());
    }
}

/**
 * Socket.sockaddr_in(port, value)
 *
 * Return sockaddr struct in bytes.
 */
static SharedPtr<Value> sock_sockaddr_in(VM * vm, Value*klass, Value* port, Value* host) {
    // TODO: return bytes.
    int port_i = port->to_int();

    SharedPtr<Value> host_s = host->to_s();
    if (host_s->is_exception()) { return host_s; }
    const std::string &ip = host_s->upcast<StrValue>()->str_value();

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
    sin.sin_port = htons(port_i);
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

    const std::string & host_s = host->upcast<StrValue>()->str_value();
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

    const std::string &s = src->upcast<StrValue>()->str_value();
    int fd = GETFD(vm, self);
    int ret;
#ifdef _WIN32
    ret = send(fd, s.c_str(), s.size(), 0);
#else
    ret = write(fd, s.c_str(), s.size());
#endif
    if (ret == -1) {
        return new ErrnoExceptionValue(get_errno());
    } else {
        return new IntValue(ret);
    }
}

/**
 * $socket.read() : Bytes
 * $socket.read(Int $len) : Bytes
 *
 * Read $len bytes from the $socket.
 */
static SharedPtr<Value> sock_read(VM * vm, const std::vector<SharedPtr<Value>>&args) {
    SharedPtr<Value> self = args.at(0);
    int fd = GETFD(vm, self.get());
    if (args.size() == 1) {
        int readed = 0;
        std::string bbuf;
        while (1) {
            char buf[4096];
            int ret;
#ifdef _WIN32
            ret = recv(fd, buf, sizeof(buf), 0);
#else
            ret = read(fd, buf, sizeof(buf));
#endif
            if (ret > 0) {
                // printf("CONT!\n");
                readed += ret;
                bbuf += std::string(buf, ret);
                continue;
            } else if (ret == 0) { // EOF
                return new BytesValue(bbuf);
            } else {
                throw new ErrnoExceptionValue(get_errno());
            }
        }
    } else if (args.size() == 2) {
        int size = args.at(1)->to_int();
        boost::scoped_array<char> buf(new char[size]);
#ifndef NDEBUG
        memset(buf.get(), 0, size);
#endif
        int ret;
#ifdef _WIN32
        ret = recv(fd, buf.get(), size, 0);
#else
        ret = read(fd, buf.get(), size);
#endif
        if (ret >= 0) {
            return new BytesValue(std::string(buf.get(), ret));
        } else {
            throw new ErrnoExceptionValue(get_errno());
        }
    } else {
        throw new ExceptionValue("Invalid argument count for Socket::Socket::read: %ld", (long int) args.size());
    }
}

/**
 * $sock.bind(Str $addr) : Undef
 *
 */
static SharedPtr<Value> sock_sock_bind(VM * vm, Value* self, Value*addr_v) {
    SharedPtr<Value> addr = addr_v->to_s();
    if (addr->is_exception()) { return addr; }

    const std::string & addr_s = addr_v->upcast<StrValue>()->str_value();
    int ret = bind(GETFD(vm, self), (const sockaddr*)addr_s.c_str(), addr_s.size());
    if (ret==0) {
        return UndefValue::instance();
    } else {
        throw new ErrnoExceptionValue(get_errno());
    }
}

static SharedPtr<Value> sock_sock_listen(VM * vm, Value* self, Value* queue_v) {
    int queue = queue_v->to_int();

    int ret = listen(GETFD(vm, self), queue);
    if (ret == 0) {
        return UndefValue::instance();
    } else {
        return new ErrnoExceptionValue(get_errno());
    }
}

static SharedPtr<Value> sock_sock_setsockopt(VM * vm, Value* self, Value* level_v, Value* optname_v, Value *optval_v) {
    int level = level_v->to_int();

    int optname = optname_v->to_int();

    const void *optval;
    socklen_t optlen;
    int n;
    if (optval_v->value_type == VALUE_TYPE_INT) {
        n = optval_v->upcast<IntValue>()->int_value();
        optval = &n;
        optlen = sizeof(n);
    } else {
        SharedPtr<Value> optval_s = optval_v->to_s();
        if (optval_s->is_exception()) { return optval_s; }
        optval = optval_s->upcast<StrValue>()->str_value().c_str();
        optlen = optval_s->upcast<StrValue>()->str_value().size();
    }

    int ret;
#ifdef _WIN32
	ret = setsockopt(GETFD(vm, self), level, optname, (char*) optval, optlen);
#else
	ret = setsockopt(GETFD(vm, self), level, optname, optval, optlen);
#endif
    if (ret == 0) {
        return UndefValue::instance();
    } else {
        return new ErrnoExceptionValue(get_errno());
    }
}

static SharedPtr<Value> sock_sock_close(VM * vm, Value* self) {
    int ret;
#ifdef _WIN32
    ret = closesocket(GETFD(vm, self));
#else
    ret = close(GETFD(vm, self));
#endif
    if (ret == 0) {
        return UndefValue::instance();
    } else {
        return new ErrnoExceptionValue(get_errno());
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
        SharedPtr<ObjectValue> new_sock = new ObjectValue(vm, vm->get_class(vm->symbol_table->get_id("Socket::Socket")), new IntValue(fd));
        SharedPtr<TupleValue> t = new TupleValue();
        t->push_back(new StrValue(std::string(namebuf, len)));
        t->push_back(new_sock);
        return t;
    } else {
        return new ErrnoExceptionValue(get_errno());
    }
}

extern "C" {

TORA_EXPORT
void Init_Socket(VM* vm) {
    // socket, bind, listen, accept, send, recv
    // "accept", "bind", "connect", "getpeername", "getsockname", "getsockopt", "listen", "recv", "send", "setsockopt",
    // "shutdown", "socket", "socketpair"

#ifdef _WIN32
    {
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);
    }
#endif

    {
        SharedPtr<ClassValue> klass = new ClassValue(vm, "Socket");
        klass->add_method("socket", new CallbackFunction(sock_socket));
        klass->add_method("sockaddr_in", new CallbackFunction(sock_sockaddr_in));
        klass->add_method("inet_aton", new CallbackFunction(sock_inet_aton));
        vm->add_class(klass);
    }

    {
        SharedPtr<ClassValue> klass = new ClassValue(vm, "Socket::Socket");
        klass->add_method("connect", new CallbackFunction(sock_sock_connect));
        klass->add_method("write", new CallbackFunction(sock_write));
        klass->add_method("read", new CallbackFunction(sock_read));
        klass->add_method("DESTROY", new CallbackFunction(sock_sock_DESTROY));
        klass->add_method("bind", new CallbackFunction(sock_sock_bind));
        klass->add_method("listen", new CallbackFunction(sock_sock_listen));
        klass->add_method("close", new CallbackFunction(sock_sock_close));
        klass->add_method("accept", new CallbackFunction(sock_sock_accept));
        klass->add_method("setsockopt", new CallbackFunction(sock_sock_setsockopt));
        vm->add_class(klass);
    }
}

}
