#ifndef TORA_NODE_H_
#define TORA_NODE_H_

#include <vector>
#include "nodes.gen.h"
#include "shared_ptr.h"
#include "prim.h"

#include <string>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

namespace tora {

class BinaryNode;
class MethodCallNode;
class NodeNode;
class IfNode;
class ForNode;
class ForEachNode;
class FuncdefNode;
class FuncallNode;
class IntNode;
class DoubleNode;
class StrNode;
class ListNode;
class TryNode;

class Node {
    PRIM_DECL();
public:
    node_type_t type;
    std::vector<SharedPtr<Node>> *list;
    typedef std::vector<SharedPtr<Node>>::iterator iterator;
    typedef std::vector<SharedPtr<Node>>::const_iterator const_iterator;
    typedef std::vector<SharedPtr<Node>>::reverse_iterator reverse_iterator;
    int lineno;

    Node(node_type_t n=NODE_UNKNOWN) :refcnt(0) {
        type = n;
        list = new std::vector<SharedPtr<Node>>();
    }
    Node(node_type_t type_, Node* node) :refcnt(0), type(type_) {
        list = new std::vector<SharedPtr<Node>>();
        list->push_back(node);
    }
    /**
     * ~Node is not a virtual for performance reason.
     * Do not override it.
     */
    virtual ~Node() { delete list; }
    const char *type_name_str() const {
        return node_type2name[this->type];
    }
    virtual void dump(int indent);
    void dump();

    Node* at(int i) {
        return list->at(i).get();
    }

    size_t size() const {
        return list->size();
    }

    template<class Y>
    Y* upcast() {
        return static_cast<Y*>(this);
    }

    iterator begin() { return list->begin(); }
    iterator end() { return list->end(); }
    const_iterator begin() const { return list->begin(); }
    const_iterator end()  const { return list->end(); }
    reverse_iterator rbegin() { return list->rbegin(); }
    reverse_iterator rend() { return list->rend(); }
    void push_back(Node* n) { list->push_back(n); }
};

class StrNode : public Node {
public:
    std::string str_value;
    StrNode(node_type_t type_, const char *str) : Node(type_), str_value(str) { }
    StrNode(node_type_t type_, const std::string &str) :Node(type_), str_value(str) { }
    void dump(int indent);
};

class RegexpNode : public Node {
public:
    std::string regexp_value;
    int flags;
    RegexpNode(node_type_t type_, const std::string &str, int flags_) :Node(type_), regexp_value(str), flags(flags_) {
    }
    void dump(int indent);
};

class IntNode: public Node {
public:
    int int_value;
    IntNode(node_type_t type_, int i): Node() {
        type = type_;
        int_value = i;
    }
    void dump(int indent);
};

class DoubleNode: public Node {
public:
    double double_value;
    DoubleNode(node_type_t type_, double d): Node() {
        type = type_;
        double_value = d;
    }
    void dump(int indent);
};

class FuncallNode: public Node {
public:
    bool is_bare;
    SharedPtr<Node>name() { return this->list->at(0); }
    SharedPtr<ListNode> args() { return this->list->at(1)->upcast<ListNode>(); }
    FuncallNode(SharedPtr<Node> name_, SharedPtr<ListNode> args_, bool bare=false) : Node(NODE_FUNCALL) {
        this->list->push_back(name_);
        this->list->push_back(args_);
        is_bare = bare;
    }
};

class FuncdefNode: public Node {
public:
    SharedPtr<Node>name() { return this->list->at(0); }
    bool have_params() {
        return this->list->at(1).get() != NULL;
    }
    SharedPtr<ListNode> params() { return this->list->at(1)->upcast<ListNode>(); }
    SharedPtr<Node>block() { return this->list->at(2); }
    FuncdefNode(SharedPtr<Node> n, SharedPtr<ListNode> p, SharedPtr<Node> b):Node(NODE_FUNCDEF) {
        this->list->push_back(n);
        this->list->push_back(p);
        this->list->push_back(b);
    }
};

class ListNode: public Node {
public:
    ListNode() : Node() { }
    ListNode(node_type_t t) : Node(t) { }
    void push_back(SharedPtr<Node> n) {
        list->push_back(n);
    }
    size_t size() { return list->size(); }
    SharedPtr<Node> at(int i) { return list->at(i); }
    SharedPtr<Node> back() { return list->back(); }
    void pop_back() { list->pop_back(); }
};

class ForNode: public Node {
public:
    ForNode(SharedPtr<Node>i, SharedPtr<Node>c, SharedPtr<Node>p, SharedPtr<Node>b): Node(NODE_FOR) {
        this->list->push_back(i);
        this->list->push_back(c);
        this->list->push_back(p);
        this->list->push_back(b);
    }
    SharedPtr<Node> initialize() { return this->list->at(0); }
    SharedPtr<Node> cond() { return this->list->at(1); }
    SharedPtr<Node> postfix() { return this->list->at(2); }
    SharedPtr<Node> block() { return this->list->at(3); }
};

class ForEachNode: public Node {
public:
    SharedPtr<Node> source() { return this->list->at(0); }
    SharedPtr<Node> vars() { return this->list->at(1); }
    SharedPtr<Node> block() { return this->list->at(2); }
    ForEachNode(SharedPtr<Node> src_, SharedPtr<Node> vars_, SharedPtr<Node> b): Node(NODE_FOREACH) {
        this->list->push_back(src_);
        this->list->push_back(vars_);
        this->list->push_back(b);
    }
};

class IfNode: public Node {
public:
    SharedPtr<Node> cond() { return this->list->at(0); }
    SharedPtr<Node> if_body() { return this->list->at(1); }
    SharedPtr<Node> else_body() { return this->list->at(2); }
    IfNode(node_type_t t, SharedPtr<Node> c, SharedPtr<Node> i, SharedPtr<Node> e) : Node(t) {
        this->list->push_back(c);
        this->list->push_back(i);
        this->list->push_back(e);
    }
};

class NodeNode: public Node {
public:
    NodeNode(node_type_t t,SharedPtr<Node>n) : Node(t) {
        this->list->push_back(n);
    }
    SharedPtr<Node> node() { return this->list->at(0); }
};

class MethodCallNode: public Node {
public:
    SharedPtr<Node>object () { return this->list->at(0); }
    SharedPtr<Node>method() { return this->list->at(1); }
    SharedPtr<ListNode> args() { return this->list->at(2)->upcast<ListNode>(); }

    MethodCallNode(SharedPtr<Node> o, SharedPtr<Node> m, SharedPtr<ListNode> a) : Node(NODE_METHOD_CALL) {
        this->list->push_back(o);
        this->list->push_back(m);
        this->list->push_back(a);
    }
};

class ClassNode : public Node {
public:
    SharedPtr<Node>     klass()  { return this->list->at(0); }
    SharedPtr<Node>     parent() { return this->list->at(1); }
    SharedPtr<ListNode> roles()  { return this->list->at(2)->upcast<ListNode>(); }
    SharedPtr<Node>     block()  { return this->list->at(3); }

    ClassNode(SharedPtr<Node> k, SharedPtr<Node>p, SharedPtr<ListNode>r, SharedPtr<Node> b) : Node(NODE_CLASS) {
        this->list->push_back(k);
        this->list->push_back(p);
        this->list->push_back(r);
        this->list->push_back(b);
    }
};


class BinaryNode: public Node {
public:
    SharedPtr<Node>left () const { return this->list->at(0); }
    SharedPtr<Node>right() const { return this->list->at(1); }
    BinaryNode(node_type_t t, SharedPtr<Node> l, SharedPtr<Node> r) : Node(t) {
        this->list->push_back(l);
        this->list->push_back(r);
    }
};

};

#endif // TORA_NODE_H_

