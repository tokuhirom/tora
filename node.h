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
class FuncdefNode;
class FuncallNode;
class IntNode;
class DoubleNode;
class StrNode;
class ArgsNode;
class RangeNode;
class ListNode;

class Node : public Prim {
public:
    node_type_t type;
    virtual ~Node() {
        // printf("%s llll \n", this->type_name_str());
    }
    const char *type_name_str() {
        return node_type2name[this->type];
    }
    virtual void dump(int indent) = 0;
    void dump();

    template<class Y>
    Y* upcast() {
        return dynamic_cast<Y*>(&(*(this)));
    }
};

class ArgsNode: public Node {
public:
    SharedPtr<ListNode> args;
    ArgsNode(node_type_t type_, SharedPtr<ListNode> args_) {
        this->type = type_;
        this->args = args_;
    }
    ~ArgsNode() { }
    void dump(int indent);
};

class StrNode : public Node {
public:
    std::string str_value;
    StrNode(node_type_t type_, const char *str) {
        this->type = type_;
        this->str_value = str;
    }
    StrNode(node_type_t type_, const std::string &str) {
        this->type = type_;
        this->str_value = str;
    }
    ~StrNode() { }
    void dump(int indent);
};

class RegexpNode : public Node {
public:
    std::string regexp_value;
    RegexpNode(node_type_t type_, const std::string &str) {
        this->type = type_;
        this->regexp_value = str;
    }
    ~RegexpNode() { }
    void dump(int indent);
};

class VoidNode: public Node {
public:
    VoidNode(node_type_t t) {
        this->type = t;
    }
    ~VoidNode() { }
    void dump(int indent);
};

class IntNode: public Node {
public:
    int int_value;
    IntNode(node_type_t type_, int i): Node() {
        type = type_;
        int_value = i;
    }
    ~IntNode() {
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
    ~DoubleNode() {
    }
    void dump(int indent);
};

class FuncallNode: public Node {
public:
    SharedPtr<Node>name;
    SharedPtr<ListNode> args;
    FuncallNode(Node * name_, SharedPtr<ListNode> args_) {
        this->type = NODE_FUNCALL;
        this->name = name_;
        this->args = args_;
    }
    ~FuncallNode() { }
    void dump(int indent);
};

class FuncdefNode: public Node {
public:
    SharedPtr<Node>name;
    SharedPtr<ListNode> params;
    SharedPtr<Node>block;
    FuncdefNode(Node *n, SharedPtr<ListNode> p, Node *b):Node() {
        this->type = NODE_FUNCDEF;
        name = n;
        params = p;
        block = b;
    }
    ~FuncdefNode() { }
    void dump(int indent);
};

class ListNode: public Node {
public:
    std::vector<SharedPtr<Node>> *list;
    ListNode() :Node() {
        type = NODE_UNKNOWN;
        list = new std::vector<SharedPtr<Node>>();
    }
    ListNode(node_type_t n) : Node() {
        type = n;
        list = new std::vector<SharedPtr<Node>>();
    }
    ~ListNode() {
        delete list;
    }
    void push_back(SharedPtr<Node> n) {
        list->push_back(n);
    }
    size_t size() { return list->size(); }
    SharedPtr<Node> at(int i) { return list->at(i); }
    SharedPtr<Node> back() { return list->back(); }
    void pop_back() { list->pop_back(); }
    void dump(int indent);
};

class ForNode: public Node {
public:
    SharedPtr<Node>initialize;
    SharedPtr<Node>cond;
    SharedPtr<Node>postfix;
    SharedPtr<Node>block;
    ForNode(SharedPtr<Node>i, SharedPtr<Node>c, SharedPtr<Node>p, SharedPtr<Node>b): Node() {
        this->type = NODE_FOR;
        initialize = i;
        cond = c;
        postfix = p;
        block = b;
    }
    ~ForNode() { }
    void dump(int indent);
};

class IfNode: public Node {
public:
    SharedPtr<Node> cond;
    SharedPtr<Node> if_body;
    SharedPtr<Node> else_body;
    IfNode(node_type_t t, SharedPtr<Node> c, SharedPtr<Node> i, SharedPtr<Node> e) : Node() {
        type = t;
        cond = c;
        if_body = i;
        else_body = e;
    }
    ~IfNode() { }
    void dump(int indent);
};

class NodeNode: public Node {
public:
    SharedPtr<Node>node;
    NodeNode(node_type_t t,SharedPtr<Node>n) : Node() {
        type = t;
        node = n;
    }
    ~NodeNode() { }
    void dump(int indent);
};

class MethodCallNode: public Node {
public:
    SharedPtr<Node>object;
    SharedPtr<Node>method;
    SharedPtr<ListNode> args;

    MethodCallNode(Node*o, Node*m, SharedPtr<ListNode> a) : Node() {
        type = NODE_METHOD_CALL;
        object = o;
        method = m;
        args = a;
    }
    ~MethodCallNode() { }
    void dump(int indent);
};

class BinaryNode: public Node {
public:
    SharedPtr<Node>left;
    SharedPtr<Node>right;
    BinaryNode(node_type_t t, SharedPtr<Node> &l, SharedPtr<Node>& r) {
        type = t;
        left = l;
        right = r;
    }
    BinaryNode(node_type_t t, Node* l, Node* r) {
        type = t;
        left = l;
        right = r;
    }
    ~BinaryNode() { }
    void dump(int indent);
};

};

#endif // TORA_NODE_H_

